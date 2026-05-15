#include "grammar.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ---------------------------------------------------------------------- */
/* Helpers                                                                */
/* ---------------------------------------------------------------------- */

static char nonterm_to_char(int n) {
    return (char)('A' + n);
}

static bool is_nonterm(int c) {
    return c >= 'A' && c <= 'Z';
}

static bool is_term(int c) {
    return c >= 'a' && c <= 'z';
}

static int fresh_nonterm(Grammar *g, int *counter) {
    (void)g;
    /* Use variables Z, Y, X, W, V, U, T, ... going backwards */
    int idx = 25 - (*counter);
    (*counter)++;
    return idx; /* 0..25 for A..Z */
}

/* ---------------------------------------------------------------------- */
/* Create                                                                 */
/* ---------------------------------------------------------------------- */

Grammar grammar_create(int start_symbol) {
    Grammar g;
    g.num_productions = 0;
    g.start_symbol = start_symbol;
    g.is_cnf = false;
    return g;
}

/* Parse a production rhs string. '\0' char in string means epsilon. */
void grammar_add_production(Grammar *g, int lhs, const char *rhs) {
    if (g->num_productions >= MAX_GRAMMAR_PRODS) return;
    Production *p = &g->prods[g->num_productions++];
    p->lhs = lhs;
    int len = (int)strlen(rhs);
    if (len == 0) {
        p->rhs[0] = '\0';
        p->rhs_len = 1;
    } else {
        for (int i = 0; i < len && i < MAX_PROD_LEN; i++) {
            p->rhs[i] = rhs[i];
        }
        p->rhs_len = len;
    }
}

/* ---------------------------------------------------------------------- */
/* CNF Conversion                                                         */
/* ---------------------------------------------------------------------- */

static void remove_epsilon_productions(Grammar *g) {
    /* Find nullable nonterminals */
    bool nullable[26] = {false};
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < g->num_productions; i++) {
            Production *p = &g->prods[i];
            int lhs = p->lhs;
            if (nullable[lhs]) continue;
            /* Check if entire rhs is nullable */
            bool all_nullable = true;
            for (int j = 0; j < p->rhs_len; j++) {
                int sym = p->rhs[j];
                if (sym == '\0') { all_nullable = true; break; }
                if (!is_nonterm(sym) || !nullable[sym - 'A']) {
                    all_nullable = false;
                    break;
                }
            }
            if (all_nullable) {
                nullable[lhs] = true;
                changed = true;
            }
        }
    }

    /* Generate all combinations with nullable nonterminals removed */
    int orig_count = g->num_productions;
    for (int i = 0; i < orig_count; i++) {
        Production *p = &g->prods[i];
        if (p->rhs_len == 1 && p->rhs[0] == '\0') continue; /* skip epsilon prods */
        /* For each subset of positions that are nullable, generate a production */
        int nullable_positions[MAX_PROD_LEN];
        int num_null = 0;
        for (int j = 0; j < p->rhs_len; j++) {
            int sym = p->rhs[j];
            if (is_nonterm(sym) && nullable[sym - 'A']) {
                nullable_positions[num_null++] = j;
            }
        }
        int num_combos = 1 << num_null;
        for (int mask = 1; mask < num_combos; mask++) {
            /* Skip the full mask (all nullable removed) if rhs length after removal is 0 */
            int new_len = 0;
            int new_rhs[MAX_PROD_LEN];
            for (int j = 0; j < p->rhs_len; j++) {
                bool removed = false;
                for (int k = 0; k < num_null; k++) {
                    if (nullable_positions[k] == j && (mask & (1 << k))) {
                        removed = true;
                        break;
                    }
                }
                if (!removed) {
                    new_rhs[new_len++] = p->rhs[j];
                }
            }
            if (new_len == 0) continue; /* skip producing epsilon if not start */
            if (g->num_productions >= MAX_GRAMMAR_PRODS) break;
            Production *np = &g->prods[g->num_productions++];
            np->lhs = p->lhs;
            memcpy(np->rhs, new_rhs, new_len * sizeof(int));
            np->rhs_len = new_len;
        }
    }

    /* Remove original epsilon productions */
    int write = 0;
    for (int i = 0; i < g->num_productions; i++) {
        if (g->prods[i].rhs_len == 1 && g->prods[i].rhs[0] == '\0') {
            /* Only keep epsilon for start symbol */
            if (g->prods[i].lhs == g->start_symbol) {
                g->prods[write++] = g->prods[i];
            }
            continue;
        }
        g->prods[write++] = g->prods[i];
    }
    g->num_productions = write;
}

static void remove_unit_productions(Grammar *g) {
    /* Compute unit closure: unit_closure[A] = set of nonterminals reachable via unit productions */
    bool unit[26][26] = {{false}};
    for (int i = 0; i < 26; i++) unit[i][i] = true;

    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < g->num_productions; i++) {
            Production *p = &g->prods[i];
            if (p->rhs_len == 1 && is_nonterm(p->rhs[0])) {
                int a = p->lhs;
                int b = p->rhs[0] - 'A';
                if (!unit[a][b]) {
                    unit[a][b] = true;
                    changed = true;
                }
            }
        }
        /* transitive closure */
        for (int i = 0; i < 26; i++) {
            for (int j = 0; j < 26; j++) {
                if (unit[i][j]) {
                    for (int k = 0; k < 26; k++) {
                        if (unit[j][k] && !unit[i][k]) {
                            unit[i][k] = true;
                            changed = true;
                        }
                    }
                }
            }
        }
    }

    /* Add non-unit productions from reachable nonterminals */
    int orig_count = g->num_productions;
    for (int a = 0; a < 26; a++) {
        for (int b = 0; b < 26; b++) {
            if (a == b) continue;
            if (unit[a][b]) {
                for (int i = 0; i < orig_count; i++) {
                    Production *p = &g->prods[i];
                    if (p->lhs == b && !(p->rhs_len == 1 && is_nonterm(p->rhs[0]))) {
                        if (g->num_productions >= MAX_GRAMMAR_PRODS) break;
                        Production *np = &g->prods[g->num_productions++];
                        np->lhs = a;
                        memcpy(np->rhs, p->rhs, p->rhs_len * sizeof(int));
                        np->rhs_len = p->rhs_len;
                    }
                }
            }
        }
    }

    /* Remove unit productions */
    int write = 0;
    for (int i = 0; i < g->num_productions; i++) {
        if (!(g->prods[i].rhs_len == 1 && is_nonterm(g->prods[i].rhs[0]))) {
            g->prods[write++] = g->prods[i];
        }
    }
    g->num_productions = write;
}

void grammar_to_cnf(Grammar *g) {
    if (g->is_cnf) return;

    /* Step 1: Add new start symbol S0 -> S if needed */
    int orig_start = g->start_symbol;
    int fresh_counter = 0;

    /* Step 2: Remove epsilon productions */
    remove_epsilon_productions(g);

    /* Step 3: Remove unit productions */
    remove_unit_productions(g);

    /* Step 4: Replace terminals in productions with len > 1 */
    for (int i = 0; i < g->num_productions; i++) {
        Production *p = &g->prods[i];
        if (p->rhs_len > 1) {
            for (int j = 0; j < p->rhs_len; j++) {
                int sym = p->rhs[j];
                if (is_term(sym)) {
                    /* Create nonterminal for this terminal */
                    int nt = fresh_nonterm(g, &fresh_counter);
                    /* Add T -> a */
                    if (g->num_productions < MAX_GRAMMAR_PRODS) {
                        Production *tp = &g->prods[g->num_productions++];
                        tp->lhs = nt;
                        tp->rhs[0] = sym;
                        tp->rhs_len = 1;
                    }
                    p->rhs[j] = nonterm_to_char(nt);
                }
            }
        }
    }

    /* Step 5: Break productions with > 2 nonterminals */
    int cur_count = g->num_productions;
    for (int i = 0; i < cur_count; i++) {
        Production *p = &g->prods[i];
        while (p->rhs_len > 2) {
            int nt = fresh_nonterm(g, &fresh_counter);
            /* New production: nt -> rhs[1] rhs[2] ... */
            if (g->num_productions < MAX_GRAMMAR_PRODS) {
                Production *np = &g->prods[g->num_productions++];
                np->lhs = nt;
                int new_len = p->rhs_len - 1;
                for (int j = 0; j < new_len && j < MAX_PROD_LEN; j++) {
                    np->rhs[j] = p->rhs[j + 1];
                }
                np->rhs_len = new_len;
            }
            /* Current production: lhs -> rhs[0] nt */
            p->rhs[1] = nonterm_to_char(nt);
            p->rhs_len = 2;
            p = &g->prods[g->num_productions - 1]; /* process the new one */
        }
    }

    g->is_cnf = true;
    g->start_symbol = orig_start;
}

/* ---------------------------------------------------------------------- */
/* CYK Algorithm                                                          */
/* ---------------------------------------------------------------------- */

bool grammar_cyk(const Grammar *g, const char *input, int len) {
    if (len == 0) {
        /* Check if start symbol can derive epsilon */
        for (int i = 0; i < g->num_productions; i++) {
            if (g->prods[i].lhs == g->start_symbol &&
                g->prods[i].rhs_len == 1 && g->prods[i].rhs[0] == '\0')
                return true;
        }
        return false;
    }

    if (len > MAX_NONTERMS) return false;

    /* table[i][j] = set of nonterminals for substring i..i+j-1 (bitset u32) */
    unsigned int table[64][64];
    memset(table, 0, sizeof(table));

    /* Base case: substrings of length 1 */
    for (int i = 0; i < len; i++) {
        for (int p = 0; p < g->num_productions; p++) {
            if (g->prods[p].rhs_len == 1 &&
                g->prods[p].rhs[0] == input[i] &&
                is_term(g->prods[p].rhs[0])) {
                table[i][1] |= (1u << g->prods[p].lhs);
            }
        }
    }

    /* DP for longer substrings */
    for (int j = 2; j <= len; j++) {
        for (int i = 0; i <= len - j; i++) {
            for (int k = 1; k < j; k++) {
                /* Check all productions A -> BC */
                unsigned int left_set = table[i][k];
                unsigned int right_set = table[i + k][j - k];
                if (left_set == 0 || right_set == 0) continue;
                for (int p = 0; p < g->num_productions; p++) {
                    if (g->prods[p].rhs_len == 2 &&
                        is_nonterm(g->prods[p].rhs[0]) &&
                        is_nonterm(g->prods[p].rhs[1])) {
                        int B = g->prods[p].rhs[0] - 'A';
                        int C = g->prods[p].rhs[1] - 'A';
                        if ((left_set & (1u << B)) && (right_set & (1u << C))) {
                            table[i][j] |= (1u << g->prods[p].lhs);
                        }
                    }
                }
            }
        }
    }

    return (table[0][len] & (1u << g->start_symbol)) != 0;
}

/* ---------------------------------------------------------------------- */
/* Generate strings up to max_length                                      */
/* ---------------------------------------------------------------------- */

typedef struct {
    char buf[256];
    int  depth;
    int  max_depth;
    void (*callback)(const char *str, void *ctx);
    void *ctx;
    const Grammar *g;
} GenState;

static void generate_rec(GenState *gs, int nt) {
    if (gs->depth >= gs->max_depth) return;
    for (int i = 0; i < gs->g->num_productions; i++) {
        const Production *p = &gs->g->prods[i];
        if (p->lhs != nt) continue;
        if (p->rhs_len == 1 && p->rhs[0] == '\0') {
            /* epsilon */
            int old_len = (int)strlen(gs->buf);
            gs->callback(gs->buf, gs->ctx);
            /* restore buffer */
            gs->buf[old_len] = '\0';
            return;
        }
        /* Try each production; track depth by rparen recursion depth */
        int old_len = (int)strlen(gs->buf);
        gs->depth++;
        if (gs->depth <= gs->max_depth) {
            /* Generate all combinations for rhs */
            /* Use iterative backtracking for simplicity: just try first nonterminal's expansions */
            /* This is a simplified generator that enumerates derivations */
            int pos = old_len;
            bool all_term = true;
            for (int j = 0; j < p->rhs_len; j++) {
                if (is_term(p->rhs[j])) {
                    gs->buf[pos++] = (char)p->rhs[j];
                    gs->buf[pos] = '\0';
                } else {
                    all_term = false;
                }
            }
            if (all_term) {
                gs->callback(gs->buf, gs->ctx);
            } else {
                /* For each nonterminal, recursively generate */
                int nt_indices[4];
                int nt_count = 0;
                for (int j = 0; j < p->rhs_len; j++) {
                    if (is_nonterm(p->rhs[j])) {
                        nt_indices[nt_count++] = j;
                    }
                }
                if (nt_count > 0) {
                    /* Simplified: expand only first nonterminal, keep structure */
                    int expand_idx = nt_indices[0];
                    int expand_nt = p->rhs[expand_idx] - 'A';
                    /* Build prefix */
                    int pref_len = 0;
                    char prefix[256];
                    for (int j = 0; j < expand_idx; j++) {
                        prefix[pref_len++] = (char)p->rhs[j];
                    }
                    /* Build suffix */
                    char suffix[256];
                    int suff_len = 0;
                    for (int j = expand_idx + 1; j < p->rhs_len; j++) {
                        suffix[suff_len++] = (char)p->rhs[j];
                    }
                    suffix[suff_len] = '\0';

                    /* For each expansion of expand_nt... */
                    for (int pi = 0; pi < gs->g->num_productions; pi++) {
                        const Production *sp = &gs->g->prods[pi];
                        if (sp->lhs != expand_nt) continue;
                        /* Add prefix */
                        int save = (int)strlen(gs->buf);
                        for (int x = 0; x < pref_len; x++)
                            gs->buf[save + x] = prefix[x];
                        gs->buf[save + pref_len] = '\0';
                        /* Add expansion */
                        int mid = (int)strlen(gs->buf);
                        for (int x = 0; x < sp->rhs_len; x++)
                            gs->buf[mid + x] = (char)sp->rhs[x];
                        gs->buf[mid + sp->rhs_len] = '\0';
                        /* Add suffix */
                        int aft = (int)strlen(gs->buf);
                        for (int x = 0; x < suff_len; x++)
                            gs->buf[aft + x] = suffix[x];
                        gs->buf[aft + suff_len] = '\0';

                        bool has_nt = false;
                        for (int x = 0; gs->buf[x]; x++)
                            if (is_nonterm(gs->buf[x])) { has_nt = true; break; }

                        if (!has_nt) {
                            gs->callback(gs->buf, gs->ctx);
                        } else {
                            /* Recursively expand remaining nonterminals */
                            /* Find first nonterminal */
                            for (int x = 0; gs->buf[x]; x++) {
                                if (is_nonterm(gs->buf[x])) {
                                    int rnt = gs->buf[x] - 'A';
                                    char rest[256];
                                    strcpy(rest, gs->buf + x + 1);
                                    gs->buf[x] = '\0';
                                    generate_rec(gs, rnt);
                                    /* put back rest */
                                    int cur = (int)strlen(gs->buf);
                                    strcpy(gs->buf + cur, rest);
                                    break;
                                }
                            }
                        }
                        gs->buf[save] = '\0';
                    }
                }
            }
        }
        gs->depth--;
        gs->buf[old_len] = '\0';
    }
}

void grammar_generate(const Grammar *g, int max_length,
                      void (*callback)(const char *str, void *ctx), void *ctx) {
    GenState gs;
    gs.buf[0] = '\0';
    gs.depth = 0;
    gs.max_depth = max_length;
    gs.callback = callback;
    gs.ctx = ctx;
    gs.g = g;
    generate_rec(&gs, g->start_symbol);
}

/* ---------------------------------------------------------------------- */
/* Print                                                                  */
/* ---------------------------------------------------------------------- */

void grammar_print(const Grammar *g) {
    printf("Grammar: start=%c, CNF=%s, %d productions\n",
           nonterm_to_char(g->start_symbol),
           g->is_cnf ? "yes" : "no",
           g->num_productions);
    for (int i = 0; i < g->num_productions; i++) {
        const Production *p = &g->prods[i];
        printf("  %c -> ", nonterm_to_char(p->lhs));
        for (int j = 0; j < p->rhs_len; j++) {
            if (p->rhs[j] == '\0')
                printf("ε");
            else
                printf("%c", p->rhs[j]);
        }
        printf("\n");
    }
}
