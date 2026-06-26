#include "chomsky.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

/* ======================================================================
 * Chomsky Hierarchy Classification (L4: Standards/Theorems)
 *
 * Chomsky (1956, 1959) established the four-level hierarchy:
 *   Type 3 (Regular)    -> Finite Automaton (DFA/NFA)
 *   Type 2 (CF)         -> Pushdown Automaton (PDA)
 *   Type 1 (CS)         -> Linear-Bounded Automaton (LBA)
 *   Type 0 (Unrestricted)-> Turing Machine
 * ====================================================================== */

static bool is_nt(int c) { return c >= 'A' && c <= 'Z'; }
static bool is_t(int c)  { return c >= 'a' && c <= 'z'; }

static void count_sym_types(const int *seq, int len,
                             int *nt_cnt, int *t_cnt) {
    *nt_cnt = 0; *t_cnt = 0;
    for (int i = 0; i < len; i++) {
        if (seq[i] == '\0') continue;
        if (is_nt(seq[i])) (*nt_cnt)++;
        else if (is_t(seq[i])) (*t_cnt)++;
    }
}

static bool is_right_linear(const Production *p) {
    int nt_cnt, t_cnt;
    count_sym_types(p->rhs, p->rhs_len, &nt_cnt, &t_cnt);
    if (nt_cnt > 1) return false;
    if (nt_cnt == 1) return is_nt(p->rhs[p->rhs_len - 1]);
    return true;
}

static bool is_left_linear(const Production *p) {
    int nt_cnt, t_cnt;
    count_sym_types(p->rhs, p->rhs_len, &nt_cnt, &t_cnt);
    if (nt_cnt > 1) return false;
    if (nt_cnt == 1) return is_nt(p->rhs[0]);
    return true;
}

ChomskyType chomsky_classify_grammar(const Grammar *g) {
    if (g->num_productions == 0) return CHOMSKY_UNKNOWN;

    /* Check if all productions are right-linear, or all are left-linear.
     * A production with no nonterminals (pure terminal or epsilon) is
     * both right-linear and left-linear, so it never causes inconsistency. */
    bool all_rl = true, all_ll = true;
    bool has_rl_only = false, has_ll_only = false;

    for (int i = 0; i < g->num_productions; i++) {
        bool rl = is_right_linear(&g->prods[i]);
        bool ll = is_left_linear(&g->prods[i]);

        if (!rl) all_rl = false;
        if (!ll) all_ll = false;
        if (rl && !ll) has_rl_only = true;
        if (!rl && ll) has_ll_only = true;
    }

    /* Type 3: consistently right-linear OR consistently left-linear.
     * "mixed" means we have both a purely right-linear and a purely
     * left-linear production, making consistent classification impossible. */
    bool mixed = has_rl_only && has_ll_only;

    if ((all_rl || all_ll) && !mixed) return CHOMSKY_TYPE_3;

    /* For our Grammar representation, all productions have a single
     * nonterminal on the LHS, so they are at most Type 2 (CF).
     * Type 0 and 1 would require more general LHS forms. */
    return CHOMSKY_TYPE_2;
}

bool chomsky_verify_cnf(const Grammar *g, char *reason, int reason_sz) {
    if (!g->is_cnf) {
        if (reason && reason_sz > 0)
            snprintf(reason, reason_sz, "Grammar not marked CNF");
        return false;
    }
    for (int i = 0; i < g->num_productions; i++) {
        const Production *p = &g->prods[i];
        if (p->rhs_len == 1 && p->rhs[0] == '\0') {
            if (p->lhs != g->start_symbol) {
                if (reason && reason_sz > 0)
                    snprintf(reason, reason_sz, "Epsilon not from start");
                return false;
            }
            continue;
        }
        if (p->rhs_len == 2 && is_nt(p->rhs[0]) && is_nt(p->rhs[1])) continue;
        if (p->rhs_len == 1 && is_t(p->rhs[0])) continue;
        if (reason && reason_sz > 0)
            snprintf(reason, reason_sz, "Bad CNF form len=%d", p->rhs_len);
        return false;
    }
    return true;
}

bool chomsky_verify_gnf(const Grammar *g, char *reason, int reason_sz) {
    for (int i = 0; i < g->num_productions; i++) {
        const Production *p = &g->prods[i];
        if (p->rhs_len == 1 && p->rhs[0] == '\0') {
            if (p->lhs != g->start_symbol) {
                if (reason && reason_sz > 0)
                    snprintf(reason, reason_sz, "Epsilon not from start");
                return false;
            }
            continue;
        }
        if (p->rhs_len == 0 || !is_t(p->rhs[0])) {
            if (reason && reason_sz > 0)
                snprintf(reason, reason_sz, "Must start with terminal");
            return false;
        }
        for (int j = 1; j < p->rhs_len; j++) {
            if (!is_nt(p->rhs[j])) {
                if (reason && reason_sz > 0)
                    snprintf(reason, reason_sz, "Pos %d must be nonterminal", j);
                return false;
            }
        }
    }
    return true;
}

bool chomsky_is_regular_language(const Grammar *g) {
    /* Self-embedding criterion:
     * A nonterminal A is self-embedding if A =>+ alpha A beta
     * with BOTH alpha and beta non-empty (alpha != epsilon AND beta != epsilon).
     * Right-linear: A =>+ w A (beta=epsilon, so NOT self-embedding).
     * Left-linear:  A =>+ A w (alpha=epsilon, so NOT self-embedding).
     * If no self-embedding nonterminals, the language is regular. */

    /* First, collect direct productions where A appears on RHS */
    bool direct_self[26] = {false};    /* A -> ...A... */
    bool embed_left[26] = {false};     /* A -> ...A... with left context non-empty */
    bool embed_right[26] = {false};    /* A -> ...A... with right context non-empty */

    for (int i = 0; i < g->num_productions; i++) {
        const Production *p = &g->prods[i];
        int lhs = p->lhs;
        for (int j = 0; j < p->rhs_len; j++) {
            if (is_nt(p->rhs[j]) && (p->rhs[j] - 'A') == lhs) {
                direct_self[lhs] = true;
                /* Check left context: anything before position j */
                for (int k = 0; k < j; k++)
                    if (p->rhs[k] != '\0') embed_left[lhs] = true;
                /* Check right context: anything after position j */
                for (int k = j + 1; k < p->rhs_len; k++)
                    if (p->rhs[k] != '\0') embed_right[lhs] = true;
            }
        }
    }

    /* Build transitive closure: derives[A][B] = A =>+ B */
    bool derives[26][26] = {{false}};
    for (int i = 0; i < g->num_productions; i++) {
        int lhs = g->prods[i].lhs;
        for (int j = 0; j < g->prods[i].rhs_len; j++) {
            if (is_nt(g->prods[i].rhs[j])) {
                derives[lhs][g->prods[i].rhs[j] - 'A'] = true;
            }
        }
    }
    /* Floyd-Warshall transitive closure */
    for (int k = 0; k < 26; k++)
        for (int i = 0; i < 26; i++)
            for (int j = 0; j < 26; j++)
                if (derives[i][k] && derives[k][j])
                    derives[i][j] = true;

    /* Check if any A has: A =>+ B =>* A with proper embedding.
     * If A =>+ B and B =>* A, and B is self-embedding, then A's
     * language is non-regular. */
    for (int a = 0; a < 26; a++) {
        if (direct_self[a] && embed_left[a] && embed_right[a])
            return false; /* Direct self-embedding */
        /* Indirect: A =>+ B, B self-embeds */
        for (int b = 0; b < 26; b++) {
            if (a == b) continue;
            if (derives[a][b] && derives[b][a]) {
                if (direct_self[b] && embed_left[b] && embed_right[b])
                    return false;
            }
        }
    }
    return true;
}

/* ======================================================================
 * L-Systems (L7 Application, L8 Advanced)
 *
 * Lindenmayer Systems (1968): parallel rewriting for modeling
 * biological growth, fractals, and procedural content.
 *
 * Types: D0L (deterministic, context-free) implemented here.
 * Applications: plant modeling (Algorithmic Botany), Koch curve,
 * Sierpinski triangle, Dragon curve.
 * ====================================================================== */

LSystem lsystem_create(int num_symbols) {
    LSystem ls;
    ls.num_prods = 0;
    ls.axiom_len = 0;
    ls.num_symbols = num_symbols;
    memset(ls.axiom, 0, sizeof(ls.axiom));
    return ls;
}

void lsystem_set_axiom(LSystem *ls, const int *axiom, int len) {
    ls->axiom_len = len;
    for (int i = 0; i < len && i < MAX_LSYM_LEN; i++)
        ls->axiom[i] = axiom[i];
}

void lsystem_add_production(LSystem *ls, int symbol,
                            const int *repl, int len) {
    if (ls->num_prods >= MAX_LSYM_PRODS) return;
    LSystemProd *p = &ls->prods[ls->num_prods++];
    p->symbol = symbol;
    p->repl_len = len;
    for (int i = 0; i < len && i < MAX_LSYM_LEN; i++)
        p->replacement[i] = repl[i];
}

int lsystem_iterate(const LSystem *ls, const int *current, int cur_len,
                    int *out, int max_out) {
    int out_len = 0;
    for (int i = 0; i < cur_len; i++) {
        int sym = current[i];
        bool found = false;
        for (int p = 0; p < ls->num_prods; p++) {
            if (ls->prods[p].symbol == sym) {
                found = true;
                for (int j = 0; j < ls->prods[p].repl_len; j++) {
                    if (out && out_len < max_out)
                        out[out_len] = ls->prods[p].replacement[j];
                    out_len++;
                }
                break;
            }
        }
        if (!found) {
            if (out && out_len < max_out) out[out_len] = sym;
            out_len++;
        }
    }
    return out_len;
}

const int* lsystem_generate(LSystem *ls, int n, int *out_len, int max_total) {
    static int buf1[4096], buf2[4096];
    int *cur = buf1, *next = buf2;
    int cur_len = ls->axiom_len;
    memcpy(cur, ls->axiom, cur_len * sizeof(int));
    for (int iter = 0; iter < n; iter++) {
        int nlen = lsystem_iterate(ls, cur, cur_len, NULL, 0);
        if (nlen > max_total) break;
        lsystem_iterate(ls, cur, cur_len, next, max_total);
        int *tmp = cur; cur = next; next = tmp;
        cur_len = nlen;
    }
    *out_len = cur_len;
    return cur;
}

/* ======================================================================
 * CFL Pumping Lemma Verification (L4: Standards/Theorems)
 *
 * If L is context-free, there exists pumping length p such that
 * every string z in L with |z| >= p can be written uvwxy where:
 *   |vwx| <= p,  vx != epsilon,
 *   and for all i >= 0: uv^i w x^i y is in L.
 *
 * For CNF grammars, p = 2^(|N|+1) suffices.
 * This implementation searches the parse table for repeated
 * nonterminals to find pumpable substrings.
 * ====================================================================== */

bool cfl_pumping_decompose(const Grammar *g, const char *input, int len,
                           CFLPumpDecomp *decomp) {
    if (!g->is_cnf || len == 0) return false;

    /* Count nonterminals for pumping length computation */
    int nt_count = 0;
    bool seen[26] = {false};
    for (int i = 0; i < g->num_productions; i++) {
        int lhs = g->prods[i].lhs;
        if (lhs >= 0 && lhs < 26 && !seen[lhs]) {
            seen[lhs] = true; nt_count++;
        }
        for (int j = 0; j < g->prods[i].rhs_len; j++) {
            if (is_nt(g->prods[i].rhs[j])) {
                int nt = g->prods[i].rhs[j] - 'A';
                if (nt >= 0 && nt < 26 && !seen[nt]) {
                    seen[nt] = true; nt_count++;
                }
            }
        }
    }

    int p = 1;
    for (int i = 0; i < nt_count + 1; i++) {
        if (p > INT_MAX / 2) { p = INT_MAX; break; }
        p *= 2;
    }
    decomp->pumping_len = p;
    if (len < p) return false;

    /* Build CYK parse table */
    unsigned int table[64][64] = {{0}};
    for (int i = 0; i < len; i++) {
        for (int pi = 0; pi < g->num_productions; pi++) {
            if (g->prods[pi].rhs_len == 1 &&
                g->prods[pi].rhs[0] == input[i] &&
                is_t(g->prods[pi].rhs[0])) {
                table[i][1] |= (1u << g->prods[pi].lhs);
            }
        }
    }
    for (int j = 2; j <= len; j++) {
        for (int i = 0; i <= len - j; i++) {
            for (int k = 1; k < j; k++) {
                unsigned int left = table[i][k];
                unsigned int right = table[i + k][j - k];
                if (!left || !right) continue;
                for (int pi = 0; pi < g->num_productions; pi++) {
                    if (g->prods[pi].rhs_len == 2 &&
                        is_nt(g->prods[pi].rhs[0]) &&
                        is_nt(g->prods[pi].rhs[1])) {
                        int B = g->prods[pi].rhs[0] - 'A';
                        int C = g->prods[pi].rhs[1] - 'A';
                        if ((left & (1u << B)) && (right & (1u << C)))
                            table[i][j] |= (1u << g->prods[pi].lhs);
                    }
                }
            }
        }
    }

    /* Find pumping decomposition: repeated nonterminal */
    for (int j = 1; j <= len && j <= p; j++) {
        for (int i = 0; i <= len - j; i++) {
            for (int a = 0; a < 26; a++) {
                if (!(table[i][j] & (1u << a))) continue;
                for (int k = 1; k < j; k++) {
                    for (int m = 1; k + m < j; m++) {
                        if (table[i + k][m] & (1u << a)) {
                            decomp->u_len = i;
                            decomp->v_len = k;
                            decomp->w_len = m;
                            decomp->x_len = j - k - m;
                            decomp->y_len = len - (i + j);
                            if (decomp->v_len > 0 || decomp->x_len > 0)
                                return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool cfl_pumping_verify(const Grammar *g, const char *input, int len,
                        const CFLPumpDecomp *decomp, int pump_count) {
    if (decomp->v_len == 0 && decomp->x_len == 0) return false;
    for (int p = 0; p <= pump_count; p++) {
        char pumped[512];
        int pos = 0;
        for (int x = 0; x < decomp->u_len; x++)
            pumped[pos++] = input[x];
        for (int rep = 0; rep < p; rep++)
            for (int x = 0; x < decomp->v_len; x++)
                pumped[pos++] = input[decomp->u_len + x];
        for (int x = 0; x < decomp->w_len; x++)
            pumped[pos++] = input[decomp->u_len + decomp->v_len + x];
        for (int rep = 0; rep < p; rep++)
            for (int x = 0; x < decomp->x_len; x++)
                pumped[pos++] = input[decomp->u_len + decomp->v_len +
                                      decomp->w_len + x];
        for (int x = 0; x < decomp->y_len; x++)
            pumped[pos++] = input[decomp->u_len + decomp->v_len +
                                  decomp->w_len + decomp->x_len + x];
        pumped[pos] = '\0';
        if (!grammar_cyk(g, pumped, pos)) return false;
    }
    return true;
}
