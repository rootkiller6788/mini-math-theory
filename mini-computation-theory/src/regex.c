#include "regex.h"
#include "nfa.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Global compiled NFA/DFA for the regex engine */
static NFA g_nfa;
static DFA g_dfa;
static bool g_compiled = false;
static int  g_alphabet_size = 26; /* a-z */

/* Character to symbol index (0..25). '.' maps to 26 specially if needed. */
static int char_to_sym(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= 'A' && c <= 'Z') return c - 'A';
    /* '.' is handled by matching any character at match time */
    return -1;
}

/* ---------------------------------------------------------------------- */
/* Regex parser                                                           */
/* ---------------------------------------------------------------------- */

typedef struct {
    const char *pattern;
    int  pos;
    int  len;
    char error[128];
    bool failed;
} ParseState;

static char peek(const ParseState *p) {
    if (p->pos < p->len) return p->pattern[p->pos];
    return '\0';
}

static char advance(ParseState *p) {
    if (p->pos < p->len) return p->pattern[p->pos++];
    return '\0';
}

static void parse_error(ParseState *p, const char *msg) {
    if (!p->failed) {
        snprintf(p->error, sizeof(p->error), "Error at pos %d: %s", p->pos, msg);
        p->failed = true;
    }
}

/* Forward declarations */
static NFA parse_regex(ParseState *p);
static NFA parse_atom(ParseState *p);

/* atom ::= char | '(' regex ')' | '.' */
static NFA parse_atom(ParseState *p) {
    char c = peek(p);
    if (c == '(') {
        advance(p); /* consume '(' */
        NFA nfa = parse_regex(p);
        if (p->failed) return nfa;
        if (peek(p) != ')') {
            parse_error(p, "Expected ')'");
            return nfa;
        }
        advance(p); /* consume ')' */
        return nfa;
    }
    if (c == '.' || c == '\\') {
        if (c == '\\') {
            advance(p);
            c = advance(p);
        } else {
            advance(p);
        }
        /* create NFA that accepts any character */
        NFA nfa = nfa_create(g_alphabet_size);
        int s0 = nfa_add_state(&nfa);
        int s1 = nfa_add_state(&nfa);
        nfa.start_state = s0;
        nfa.accept[s1] = true;
        nfa.accept_state = s1;
        if (c == '.') {
            for (int i = 0; i < g_alphabet_size; i++) {
                nfa_add_transition(&nfa, s0, i, s1);
            }
        } else {
            int sym = char_to_sym(c);
            if (sym < 0) {
                parse_error(p, "Invalid character after \\");
                return nfa;
            }
            nfa_add_transition(&nfa, s0, sym, s1);
        }
        return nfa;
    }
    if (c == '\0' || c == ')' || c == '|' || c == '*' || c == '+' || c == '?') {
        parse_error(p, "Unexpected character");
        NFA nfa = nfa_create(g_alphabet_size);
        return nfa;
    }
    /* literal character */
    advance(p);
    return nfa_from_symbol(g_alphabet_size, char_to_sym(c));
}

/* factor ::= atom ('*' | '+' | '?')? */
static NFA parse_factor(ParseState *p) {
    NFA a = parse_atom(p);
    if (p->failed) return a;

    char c = peek(p);
    if (c == '*') {
        advance(p);
        return nfa_thompson_star(&a);
    }
    if (c == '+') {
        advance(p);
        /* a+ = a a* */
        NFA b = nfa_thompson_star(&a);
        NFA result = nfa_thompson_concat(&a, &b);
        return result;
    }
    if (c == '?') {
        advance(p);
        /* a? = epsilon | a */
        NFA eps = nfa_create(g_alphabet_size);
        int s0 = nfa_add_state(&eps);
        int s1 = nfa_add_state(&eps);
        eps.start_state = s0;
        eps.accept[s1] = true;
        eps.accept_state = s1;
        nfa_add_epsilon(&eps, s0, s1);
        NFA result = nfa_thompson_union(&eps, &a);
        return result;
    }
    return a;
}

/* term ::= factor+   (concatenation) */
static NFA parse_term(ParseState *p) {
    NFA result = parse_factor(p);
    if (p->failed) return result;

    while (!p->failed) {
        char c = peek(p);
        if (c == '\0' || c == ')' || c == '|') break;
        NFA next = parse_factor(p);
        if (p->failed) break;
        NFA concat = nfa_thompson_concat(&result, &next);
        result = concat;
    }
    return result;
}

/* regex ::= term ('|' term)* */
static NFA parse_regex(ParseState *p) {
    NFA result = parse_term(p);
    if (p->failed) return result;

    while (peek(p) == '|') {
        advance(p); /* consume '|' */
        NFA right = parse_term(p);
        if (p->failed) break;
        NFA uni = nfa_thompson_union(&result, &right);
        result = uni;
    }
    return result;
}

/* Top-level parse */
static NFA parse_pattern(const char *pattern, ParseState *ps) {
    ps->pattern = pattern;
    ps->pos = 0;
    ps->len = (int)strlen(pattern);
    ps->failed = false;
    ps->error[0] = '\0';
    return parse_regex(ps);
}

/* ---------------------------------------------------------------------- */
/* Public API                                                             */
/* ---------------------------------------------------------------------- */

bool regex_compile(const char *pattern, char *err_msg, int err_size) {
    ParseState ps;
    g_nfa = parse_pattern(pattern, &ps);
    if (ps.failed) {
        if (err_msg && err_size > 0) {
            strncpy(err_msg, ps.error, err_size - 1);
            err_msg[err_size - 1] = '\0';
        }
        g_compiled = false;
        return false;
    }
    /* Also require that all input was consumed */
    if (ps.pos < ps.len) {
        if (err_msg && err_size > 0) {
            snprintf(err_msg, err_size, "Trailing characters at position %d", ps.pos);
        }
        g_compiled = false;
        return false;
    }
    /* Convert NFA to DFA */
    g_dfa = nfa_to_dfa(&g_nfa);
    g_compiled = true;
    return true;
}

bool regex_match(const char *text) {
    if (!g_compiled) return false;
    int len = (int)strlen(text);
    /* Convert text to symbol array */
    int input[256];
    for (int i = 0; i < len && i < 256; i++) {
        input[i] = char_to_sym(text[i]);
        if (input[i] < 0) return false;
    }
    return dfa_simulate(&g_dfa, input, len);
}

bool regex_match_partial(const char *text, int *start, int *end) {
    if (!g_compiled) return false;
    int tlen = (int)strlen(text);

    /* Try each starting position */
    for (int s = 0; s < tlen; s++) {
        int state = g_dfa.start_state;
        int match_end = -1;
        for (int i = s; i < tlen; i++) {
            int sym = char_to_sym(text[i]);
            if (sym < 0) break;
            state = g_dfa.transitions[state][sym];
            if (state == DFA_DEAD_STATE) break;
            if (g_dfa.accept[state]) {
                match_end = i + 1;
            }
        }
        if (match_end >= 0) {
            *start = s;
            *end = match_end;
            return true;
        }
    }
    return false;
}
