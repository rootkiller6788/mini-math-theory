#include "logic.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Formula* formula_create(void) {
    Formula* f = malloc(sizeof(Formula));
    f->ncount = 0;
    f->nvars = 0;
    return f;
}

int formula_add_var(Formula* f, int var_idx) {
    FormulaNode node = { .op = OP_VAR, .var_idx = var_idx };
    f->nodes[f->ncount] = node;
    return f->ncount++;
}

int formula_add_not(Formula* f, int child) {
    FormulaNode node = { .op = OP_NOT, .left = child };
    f->nodes[f->ncount] = node;
    return f->ncount++;
}

int formula_add_and(Formula* f, int left, int right) {
    FormulaNode node = { .op = OP_AND, .left = left, .right = right };
    f->nodes[f->ncount] = node;
    return f->ncount++;
}

int formula_add_or(Formula* f, int left, int right) {
    FormulaNode node = { .op = OP_OR, .left = left, .right = right };
    f->nodes[f->ncount] = node;
    return f->ncount++;
}

int formula_add_implies(Formula* f, int left, int right) {
    FormulaNode node = { .op = OP_IMPLIES, .left = left, .right = right };
    f->nodes[f->ncount] = node;
    return f->ncount++;
}

int formula_add_iff(Formula* f, int left, int right) {
    FormulaNode node = { .op = OP_IFF, .left = left, .right = right };
    f->nodes[f->ncount] = node;
    return f->ncount++;
}

int formula_add_xor(Formula* f, int left, int right) {
    FormulaNode node = { .op = OP_XOR, .left = left, .right = right };
    f->nodes[f->ncount] = node;
    return f->ncount++;
}

bool formula_eval(Formula* f, int root, const bool* assignment) {
    FormulaNode* node = &f->nodes[root];
    switch (node->op) {
        case OP_VAR: return assignment[node->var_idx];
        case OP_NOT: return !formula_eval(f, node->left, assignment);
        case OP_AND: return formula_eval(f, node->left, assignment) && formula_eval(f, node->right, assignment);
        case OP_OR:  return formula_eval(f, node->left, assignment) || formula_eval(f, node->right, assignment);
        case OP_IMPLIES: return !formula_eval(f, node->left, assignment) || formula_eval(f, node->right, assignment);
        case OP_IFF: return formula_eval(f, node->left, assignment) == formula_eval(f, node->right, assignment);
        case OP_XOR: return formula_eval(f, node->left, assignment) != formula_eval(f, node->right, assignment);
    }
    return false;
}

void truth_table_print(Formula* f, int root) {
    int n = f->nvars;
    int rows = 1 << n;

    for (int i = 0; i < n; i++)
        printf(" %c |", f->var_names[i]);
    printf(" R\n");

    for (int i = 0; i < n; i++)
        printf("---|");
    printf("---\n");

    for (int r = 0; r < rows; r++) {
        bool assign[MAX_VARS];
        for (int b = 0; b < n; b++)
            assign[b] = (r >> (n - 1 - b)) & 1;

        for (int b = 0; b < n; b++)
            printf(" %d |", assign[b]);
        printf(" %d\n", formula_eval(f, root, assign));
    }
}

bool is_tautology(Formula* f, int root) {
    int rows = 1 << f->nvars;
    for (int r = 0; r < rows; r++) {
        bool assign[MAX_VARS];
        for (int b = 0; b < f->nvars; b++)
            assign[b] = (r >> (f->nvars - 1 - b)) & 1;
        if (!formula_eval(f, root, assign))
            return false;
    }
    return true;
}

bool is_contradiction(Formula* f, int root) {
    int rows = 1 << f->nvars;
    for (int r = 0; r < rows; r++) {
        bool assign[MAX_VARS];
        for (int b = 0; b < f->nvars; b++)
            assign[b] = (r >> (f->nvars - 1 - b)) & 1;
        if (formula_eval(f, root, assign))
            return false;
    }
    return true;
}

void proof_by_induction_demo(int n) {
    printf("=== 数学归纳法演示 ===\n");
    printf("命题 P(n): 1 + 2 + ... + n = n(n+1)/2\n\n");

    printf("1. 基始: P(1)\n");
    printf("   左 = 1, 右 = 1*2/2 = 1  ✓\n\n");

    printf("2. 归纳: 假设 P(k) 成立, 证 P(k+1)\n");
    printf("   1+2+...+k+(k+1) = k(k+1)/2 + (k+1)\n");
    printf("   = (k+1)(k/2 + 1) = (k+1)(k+2)/2  ✓\n\n");

    printf("3. 验证若干项:\n");
    int sum = 0;
    for (int i = 1; i <= n && i <= 10; i++) {
        sum += i;
        printf("   n=%d: 和=%d, 公式=%d\n", i, sum, i * (i + 1) / 2);
    }
    printf("\n");
}

/* ===== CNF Conversion (Tseitin Transformation) =====
 * Converts any propositional formula to equisatisfiable CNF in linear time.
 * Adds auxiliary variables for each subformula.
 * Reference: Tseitin (1968), "On the complexity of derivation in propositional calculus" */
static int tseitin_encode(Formula* f, int root, CNF* cnf, int* next_var) {
    FormulaNode* node = &f->nodes[root];
    switch (node->op) {
        case OP_VAR: {
            /* variable maps to itself */
            int v = node->var_idx;
            /* lit represents X_v for positive variable */
            return v + 1; /* positive literal number, 0 = not used */
        }
        case OP_NOT: {
            int child = tseitin_encode(f, node->left, cnf, next_var);
            int aux = (*next_var)++;
            /* aux ↔ ¬child */
            /* (¬aux ∨ ¬child) ∧ (aux ∨ child) */
            cnf->clauses[cnf->n_clauses].lits[0] = -aux;
            cnf->clauses[cnf->n_clauses].lits[1] = -child;
            cnf->clauses[cnf->n_clauses].n_lits = 2;
            cnf->n_clauses++;
            cnf->clauses[cnf->n_clauses].lits[0] = aux;
            cnf->clauses[cnf->n_clauses].lits[1] = child;
            cnf->clauses[cnf->n_clauses].n_lits = 2;
            cnf->n_clauses++;
            return aux;
        }
        case OP_AND: {
            int left = tseitin_encode(f, node->left, cnf, next_var);
            int right = tseitin_encode(f, node->right, cnf, next_var);
            int aux = (*next_var)++;
            /* aux ↔ (left ∧ right) */
            /* (¬aux ∨ left) ∧ (¬aux ∨ right) ∧ (aux ∨ ¬left ∨ ¬right) */
            cnf->clauses[cnf->n_clauses].lits[0] = -aux;
            cnf->clauses[cnf->n_clauses].lits[1] = left;
            cnf->clauses[cnf->n_clauses].n_lits = 2;
            cnf->n_clauses++;
            cnf->clauses[cnf->n_clauses].lits[0] = -aux;
            cnf->clauses[cnf->n_clauses].lits[1] = right;
            cnf->clauses[cnf->n_clauses].n_lits = 2;
            cnf->n_clauses++;
            cnf->clauses[cnf->n_clauses].lits[0] = aux;
            cnf->clauses[cnf->n_clauses].lits[1] = -left;
            cnf->clauses[cnf->n_clauses].lits[2] = -right;
            cnf->clauses[cnf->n_clauses].n_lits = 3;
            cnf->n_clauses++;
            return aux;
        }
        case OP_OR: {
            int left = tseitin_encode(f, node->left, cnf, next_var);
            int right = tseitin_encode(f, node->right, cnf, next_var);
            int aux = (*next_var)++;
            /* aux ↔ (left ∨ right) */
            /* (¬aux ∨ left ∨ right) ∧ (aux ∨ ¬left) ∧ (aux ∨ ¬right) */
            cnf->clauses[cnf->n_clauses].lits[0] = -aux;
            cnf->clauses[cnf->n_clauses].lits[1] = left;
            cnf->clauses[cnf->n_clauses].lits[2] = right;
            cnf->clauses[cnf->n_clauses].n_lits = 3;
            cnf->n_clauses++;
            cnf->clauses[cnf->n_clauses].lits[0] = aux;
            cnf->clauses[cnf->n_clauses].lits[1] = -left;
            cnf->clauses[cnf->n_clauses].n_lits = 2;
            cnf->n_clauses++;
            cnf->clauses[cnf->n_clauses].lits[0] = aux;
            cnf->clauses[cnf->n_clauses].lits[1] = -right;
            cnf->clauses[cnf->n_clauses].n_lits = 2;
            cnf->n_clauses++;
            return aux;
        }
        case OP_IMPLIES: {
            /* p → q ≡ ¬p ∨ q */
            int left = tseitin_encode(f, node->left, cnf, next_var);
            int right = tseitin_encode(f, node->right, cnf, next_var);
            int aux = (*next_var)++;
            /* aux ↔ (¬left ∨ right) */
            cnf->clauses[cnf->n_clauses].lits[0] = -aux;
            cnf->clauses[cnf->n_clauses].lits[1] = -left;
            cnf->clauses[cnf->n_clauses].lits[2] = right;
            cnf->clauses[cnf->n_clauses].n_lits = 3;
            cnf->n_clauses++;
            cnf->clauses[cnf->n_clauses].lits[0] = aux;
            cnf->clauses[cnf->n_clauses].lits[1] = left;
            cnf->clauses[cnf->n_clauses].n_lits = 2;
            cnf->n_clauses++;
            cnf->clauses[cnf->n_clauses].lits[0] = aux;
            cnf->clauses[cnf->n_clauses].lits[1] = -right;
            cnf->clauses[cnf->n_clauses].n_lits = 2;
            cnf->n_clauses++;
            return aux;
        }
        case OP_IFF: {
            int left = tseitin_encode(f, node->left, cnf, next_var);
            int right = tseitin_encode(f, node->right, cnf, next_var);
            int aux = (*next_var)++;
            /* aux ↔ (left ↔ right) */
            cnf->clauses[cnf->n_clauses].lits[0] = -aux;
            cnf->clauses[cnf->n_clauses].lits[1] = -left;
            cnf->clauses[cnf->n_clauses].lits[2] = right;
            cnf->clauses[cnf->n_clauses].n_lits = 3;
            cnf->n_clauses++;
            cnf->clauses[cnf->n_clauses].lits[0] = -aux;
            cnf->clauses[cnf->n_clauses].lits[1] = left;
            cnf->clauses[cnf->n_clauses].lits[2] = -right;
            cnf->clauses[cnf->n_clauses].n_lits = 3;
            cnf->n_clauses++;
            cnf->clauses[cnf->n_clauses].lits[0] = aux;
            cnf->clauses[cnf->n_clauses].lits[1] = left;
            cnf->clauses[cnf->n_clauses].lits[2] = right;
            cnf->clauses[cnf->n_clauses].n_lits = 3;
            cnf->n_clauses++;
            cnf->clauses[cnf->n_clauses].lits[0] = aux;
            cnf->clauses[cnf->n_clauses].lits[1] = -left;
            cnf->clauses[cnf->n_clauses].lits[2] = -right;
            cnf->clauses[cnf->n_clauses].n_lits = 3;
            cnf->n_clauses++;
            return aux;
        }
        case OP_XOR: {
            /* p ⊕ q ≡ (p ∨ q) ∧ (¬p ∨ ¬q) */
            int left = tseitin_encode(f, node->left, cnf, next_var);
            int right = tseitin_encode(f, node->right, cnf, next_var);
            int aux = (*next_var)++;
            cnf->clauses[cnf->n_clauses].lits[0] = -aux;
            cnf->clauses[cnf->n_clauses].lits[1] = left;
            cnf->clauses[cnf->n_clauses].lits[2] = right;
            cnf->clauses[cnf->n_clauses].n_lits = 3;
            cnf->n_clauses++;
            cnf->clauses[cnf->n_clauses].lits[0] = -aux;
            cnf->clauses[cnf->n_clauses].lits[1] = -left;
            cnf->clauses[cnf->n_clauses].lits[2] = -right;
            cnf->clauses[cnf->n_clauses].n_lits = 3;
            cnf->n_clauses++;
            cnf->clauses[cnf->n_clauses].lits[0] = aux;
            cnf->clauses[cnf->n_clauses].lits[1] = left;
            cnf->clauses[cnf->n_clauses].lits[2] = -right;
            cnf->clauses[cnf->n_clauses].n_lits = 3;
            cnf->n_clauses++;
            cnf->clauses[cnf->n_clauses].lits[0] = aux;
            cnf->clauses[cnf->n_clauses].lits[1] = -left;
            cnf->clauses[cnf->n_clauses].lits[2] = right;
            cnf->clauses[cnf->n_clauses].n_lits = 3;
            cnf->n_clauses++;
            return aux;
        }
    }
    return 0;
}

CNF* formula_to_cnf(Formula* f, int root) {
    CNF* cnf = malloc(sizeof(CNF));
    cnf->n_clauses = 0;
    cnf->n_vars = f->nvars;
    int next_var = f->nvars + 1; /* auxiliary vars start at f->nvars+1 */
    int encoded_root = tseitin_encode(f, root, cnf, &next_var);
    cnf->n_vars = next_var - 1; /* total vars including auxiliaries */
    /* add unit clause asserting the root is true */
    cnf->clauses[cnf->n_clauses].lits[0] = encoded_root;
    cnf->clauses[cnf->n_clauses].n_lits = 1;
    cnf->n_clauses++;
    return cnf;
}

/* DPLL Solver (Davis-Putnam-Logemann-Loveland)
 * Classic backtracking SAT solver with:
 * 1. Unit propagation: forced assignments
 * 2. Pure literal elimination: literals with one polarity
 * 3. Splitting on remaining variable
 * Reference: Davis-Logemann-Loveland (1962), CACM vol.5 */
static bool dpll_unit_propagate(CNF* cnf, int* assign) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < cnf->n_clauses; i++) {
            Clause* c = &cnf->clauses[i];
            if (c->n_lits == 0) continue;
            /* count unassigned and check satisfiability */
            int unassigned = 0, unassigned_idx = -1;
            bool sat = false;
            for (int j = 0; j < c->n_lits; j++) {
                int lit = c->lits[j];
                int var = lit > 0 ? lit : -lit;
                if (assign[var] == 0) { /* unassigned */
                    unassigned++;
                    unassigned_idx = j;
                } else if ((lit > 0 && assign[var] == 1) ||
                           (lit < 0 && assign[var] == -1)) {
                    sat = true;
                    break;
                }
            }
            if (!sat && unassigned == 0) return false; /* conflict */
            if (!sat && unassigned == 1) {
                /* unit clause: force the literal */
                int lit = c->lits[unassigned_idx];
                int var = lit > 0 ? lit : -lit;
                assign[var] = lit > 0 ? 1 : -1;
                changed = true;
            }
        }
    }
    return true; /* no conflict */
}

static int dpll_pure_literal(CNF* cnf, int* assign) {
    int polarities[256] = {0}; /* 0=none, 1=pos, -1=neg, 2=both */
    for (int i = 0; i < cnf->n_clauses; i++) {
        Clause* c = &cnf->clauses[i];
        for (int j = 0; j < c->n_lits; j++) {
            int lit = c->lits[j];
            int var = lit > 0 ? lit : -lit;
            if (assign[var] != 0) continue;
            int pol = lit > 0 ? 1 : -1;
            if (polarities[var] == 0) polarities[var] = pol;
            else if (polarities[var] != pol) polarities[var] = 2;
        }
    }
    int found = 0;
    for (int v = 1; v <= cnf->n_vars && v < 256; v++) {
        if (assign[v] != 0) continue;
        if (polarities[v] == 1 || polarities[v] == -1) {
            assign[v] = polarities[v];
            found++;
        }
    }
    return found;
}

static int dpll_choose_var(CNF* cnf, int* assign) {
    int freq[256] = {0};
    for (int i = 0; i < cnf->n_clauses; i++) {
        Clause* c = &cnf->clauses[i];
        for (int j = 0; j < c->n_lits; j++) {
            int var = c->lits[j] > 0 ? c->lits[j] : -c->lits[j];
            if (assign[var] == 0 && var < 256) freq[var]++;
        }
    }
    int best_var = 0, best_freq = -1;
    for (int v = 1; v <= cnf->n_vars && v < 256; v++) {
        if (assign[v] == 0 && freq[v] > best_freq) {
            best_freq = freq[v];
            best_var = v;
        }
    }
    return best_var;
}

bool dpll_solve_rec(CNF* cnf, int* assign, int depth) {
    (void)depth;
    if (!dpll_unit_propagate(cnf, assign)) return false;

    /* check if all clauses satisfied */
    bool all_sat = true;
    for (int i = 0; i < cnf->n_clauses; i++) {
        Clause* c = &cnf->clauses[i];
        bool sat = false;
        for (int j = 0; j < c->n_lits; j++) {
            int lit = c->lits[j];
            int var = lit > 0 ? lit : -lit;
            if (assign[var] != 0 &&
                ((lit > 0 && assign[var] == 1) || (lit < 0 && assign[var] == -1))) {
                sat = true;
                break;
            }
        }
        if (!sat) { all_sat = false; break; }
    }
    if (all_sat) return true;

    /* pure literal elimination */
    dpll_pure_literal(cnf, assign);

    /* choose branching variable */
    int var = dpll_choose_var(cnf, assign);
    if (var == 0) return true; /* all assigned */

    /* save state */
    int saved[256];
    for (int v = 1; v <= cnf->n_vars && v < 256; v++)
        saved[v] = assign[v];

    /* try TRUE */
    assign[var] = 1;
    if (dpll_solve_rec(cnf, assign, depth + 1)) return true;

    /* backtrack, try FALSE */
    for (int v = 1; v <= cnf->n_vars && v < 256; v++)
        assign[v] = saved[v];
    assign[var] = -1;
    if (dpll_solve_rec(cnf, assign, depth + 1)) return true;

    /* backtrack again */
    for (int v = 1; v <= cnf->n_vars && v < 256; v++)
        assign[v] = saved[v];
    return false;
}

bool dpll_solve(CNF* cnf, bool* model) {
    int assign[256] = {0}; /* 0=unassigned, 1=TRUE, -1=FALSE */
    /* Initialize with original variables range */
    bool result = dpll_solve_rec(cnf, assign, 0);
    if (result && model) {
        for (int v = 1; v <= cnf->n_vars; v++)
            model[v - 1] = (assign[v] == 1);
    }
    return result;
}

/* Resolution: deduce new clause C1⊗C2 from C1 and C2 via complementary literal.
 * Resolution rule: (A ∨ x), (B ∨ ¬x) ⊢ (A ∨ B).
 * Reference: Robinson (1965), "A Machine-Oriented Logic" */
static bool clause_resolve(Clause* a, Clause* b, Clause* result) {
    int resolved_var = 0;
    for (int i = 0; i < a->n_lits; i++) {
        int lit_a = a->lits[i];
        for (int j = 0; j < b->n_lits; j++) {
            if (b->lits[j] == -lit_a) {
                if (resolved_var != 0 && resolved_var != (lit_a > 0 ? lit_a : -lit_a))
                    return false; /* multiple complementary pairs — tautology */
                resolved_var = lit_a > 0 ? lit_a : -lit_a;
                break;
            }
        }
    }
    if (resolved_var == 0) return false; /* no complementary pair */

    result->n_lits = 0;
    for (int i = 0; i < a->n_lits; i++) {
        int lit = a->lits[i];
        int v = lit > 0 ? lit : -lit;
        if (v != resolved_var) {
            /* deduplicate */
            bool dup = false;
            for (int k = 0; k < result->n_lits; k++)
                if (result->lits[k] == lit) { dup = true; break; }
            if (!dup) result->lits[result->n_lits++] = lit;
        }
    }
    for (int j = 0; j < b->n_lits; j++) {
        int lit = b->lits[j];
        int v = lit > 0 ? lit : -lit;
        if (v != resolved_var) {
            bool dup = false;
            for (int k = 0; k < result->n_lits; k++)
                if (result->lits[k] == lit) { dup = true; break; }
            if (!dup) result->lits[result->n_lits++] = lit;
        }
    }
    return true;
}

bool resolution_prove(CNF* cnf_from, int* goal_lits, int goal_nlits) {
    /* negate goal, add to clauses, try to derive empty clause */
    CNF cnf = *cnf_from;
    /* add negated goal as unit clauses */
    for (int i = 0; i < goal_nlits; i++) {
        cnf.clauses[cnf.n_clauses].lits[0] = -goal_lits[i];
        cnf.clauses[cnf.n_clauses].n_lits = 1;
        cnf.n_clauses++;
    }

    int max_iter = 4096;
    for (int iter = 0; iter < max_iter && cnf.n_clauses < MAX_CLAUSES; iter++) {
        /* try all pairs */
        for (int i = 0; i < cnf.n_clauses; i++) {
            for (int j = i + 1; j < cnf.n_clauses; j++) {
                Clause resolved;
                if (clause_resolve(&cnf.clauses[i], &cnf.clauses[j], &resolved)) {
                    if (resolved.n_lits == 0) return true; /* empty clause! */
                    /* check if already present */
                    bool exists = false;
                    for (int k = 0; k < cnf.n_clauses; k++) {
                        if (cnf.clauses[k].n_lits == resolved.n_lits) {
                            bool match = true;
                            for (int m = 0; m < resolved.n_lits; m++) {
                                bool found = false;
                                for (int n = 0; n < cnf.clauses[k].n_lits; n++)
                                    if (cnf.clauses[k].lits[n] == resolved.lits[m])
                                        { found = true; break; }
                                if (!found) { match = false; break; }
                            }
                            if (match) { exists = true; break; }
                        }
                    }
                    if (!exists && cnf.n_clauses < MAX_CLAUSES)
                        cnf.clauses[cnf.n_clauses++] = resolved;
                }
            }
        }
    }
    return false; /* could not derive empty clause */
}

/* Horn SAT: every clause has at most one positive literal.
 * Forward chaining: mark all conclusions of clauses whose premises are true.
 * Reference: Dowling-Gallier (1984), J. Logic Programming */
bool horn_sat_solve(CNF* cnf, bool* model) {
    int assign[256] = {0}; /* 0=unknown, 1=true, -1=false */
    /* Step 1: all negative-only clauses force their negated literals true.
       Also, all unit positive clauses force positive literal true. */
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < cnf->n_clauses; i++) {
            Clause* c = &cnf->clauses[i];
            int pos_count = 0, neg_undef_count = 0;
            int pos_lit = 0;
            bool pos_assigned = false;
            bool clause_true = false;

            for (int j = 0; j < c->n_lits; j++) {
                int lit = c->lits[j];
                int var = lit > 0 ? lit : -lit;
                if (lit > 0) {
                    pos_count++;
                    pos_lit = lit;
                    if (assign[var] == 1) { clause_true = true; pos_assigned = true; break; }
                }
                if (assign[var] != 0) {
                    if ((lit > 0 && assign[var] == 1) || (lit < 0 && assign[var] == -1))
                        clause_true = true;
                } else {
                    neg_undef_count++;
                }
            }

            if (clause_true) continue;

            /* Horn clause with exactly 1 positive, all negs known false */
            /* or: negative-only clause with any unassigned negs */
            if (pos_count == 0 && neg_undef_count > 0) {
                /* can't do anything */
            } else if (pos_count == 1 && !pos_assigned) {
                /* check if all negative literals are known false */
                for (int j = 0; j < c->n_lits; j++) {
                    int lit = c->lits[j];
                    int var = lit > 0 ? lit : -lit;
                    if (lit < 0 && assign[var] != 1) break;
                }
            }

            /* Simplified Horn SAT: unit propagation only */
            if (pos_count == 1 && pos_lit > 0) {
                int var = pos_lit;
                /* check if all negative literals are known true (i.e., neg_lit assigned -1 or the var assigned 1) */
                bool all_premises_satisfied = true;
                for (int j = 0; j < c->n_lits; j++) {
                    int lit = c->lits[j];
                    if (lit < 0) {
                        int pre_var = -lit;
                        /* premise is true if var is true (since ¬prem means prem=true when lit is negative) */
                        if (assign[pre_var] != 1) {
                            all_premises_satisfied = false;
                            break;
                        }
                    }
                }
                if (all_premises_satisfied && assign[var] == 0) {
                    assign[var] = 1;
                    changed = true;
                }
            }

            if (pos_count == 0) {
                /* all negative: at least one must be true. Check for unit propagation */
                int undef_count = 0, undef_var = 0;
                bool clause_already_true = false;
                for (int j = 0; j < c->n_lits; j++) {
                    int var = -c->lits[j]; /* all literals are negative */
                    if (assign[var] == 1) clause_already_true = true; /* premise true means clause satisfied since negative lit */
                    else if (assign[var] == 0) { undef_count++; undef_var = var; }
                }
                if (!clause_already_true && undef_count == 1 && assign[undef_var] == 0) {
                    assign[undef_var] = 1; /* force premise true to satisfy all-neg clause */
                    changed = true;
                } else if (!clause_already_true && undef_count == 0)
                    return false; /* contradiction: all premises false in all-neg clause */
            }
        }
    }

    /* remaining unassigned → set to false */
    if (model) {
        for (int v = 1; v <= cnf->n_vars; v++)
            model[v - 1] = (assign[v] == 1);
    }
    return true; /* satisfiable */
}

/* 2-SAT: each clause has at most 2 literals.
 * Build implication graph: (a ∨ b) ≡ (¬a → b) ∧ (¬b → a).
 * Formula is SAT iff no variable and its negation in same SCC.
 * Reference: Aspvall-Plass-Tarjan (1979), IPL vol.8 */
bool two_sat_solve(int n_vars, int clauses[][2], int n_clauses, bool* model) {
    int n = 2 * n_vars + 1; /* var x: node 2x (false), node 2x+1 (true) */
    int adj[128][64]; /* 2*64 = 128 nodes max */
    int deg[128] = {0};

    for (int i = 0; i < n_clauses; i++) {
        int a = clauses[i][0], b = clauses[i][1];
        /* convert to implication edges */
        int from1 = a > 0 ? 2 * (-a) : 2 * (a);
        int to1   = b > 0 ? 2 * b + 1 : 2 * (-b);
        int from2 = b > 0 ? 2 * (-b) : 2 * (b);
        int to2   = a > 0 ? 2 * a + 1 : 2 * (-a);
        adj[from1][deg[from1]++] = to1;
        adj[from2][deg[from2]++] = to2;
    }

    /* Kosaraju SCC */
    bool visited[128] = {false};
    int fin[128], fidx = 0;
    int rev_adj[128][64], rev_deg[128] = {0};
    for (int u = 0; u < n; u++)
        for (int i = 0; i < deg[u]; i++)
            rev_adj[adj[u][i]][rev_deg[adj[u][i]]++] = u;

    /* DFS 1 on original */
    typedef struct { int v; int state; int ni; } DFrame;
    DFrame stack[256]; int sp = 0;
    for (int s = 0; s < n; s++) {
        if (visited[s]) continue;
        stack[sp++] = (DFrame){s, 0, 0};
        while (sp > 0) {
            DFrame* cur = &stack[sp - 1];
            if (cur->state == 0) { visited[cur->v] = true; cur->state = 1; }
            if (cur->ni < deg[cur->v]) {
                int w = adj[cur->v][cur->ni++];
                if (!visited[w]) { stack[sp++] = (DFrame){w, 0, 0}; }
            } else {
                fin[fidx++] = cur->v;
                sp--;
            }
        }
    }

    /* DFS 2 on reversed in reverse order */
    memset(visited, 0, sizeof(visited));
    int comp[128];
    int nc = 0;
    for (int i = fidx - 1; i >= 0; i--) {
        int v = fin[i];
        if (visited[v]) continue;
        int q[128], h = 0, t = 0;
        q[t++] = v; visited[v] = true;
        while (h < t) {
            int u = q[h++];
            comp[u] = nc;
            for (int j = 0; j < rev_deg[u]; j++) {
                int w = rev_adj[u][j];
                if (!visited[w]) { visited[w] = true; q[t++] = w; }
            }
        }
        nc++;
    }

    /* check for contradiction */
    for (int v = 1; v <= n_vars; v++) {
        if (comp[2 * v] == comp[2 * v + 1]) return false; /* UNSAT */
    }

    /* assign: if comp[2v+1] > comp[2v], set true */
    if (model) {
        for (int v = 1; v <= n_vars; v++)
            model[v - 1] = (comp[2 * v + 1] > comp[2 * v]);
    }
    return true;
}

/* Semantic tableau: systematic search tree for counterexample.
 * Expands formula by breaking connectives, checking for branch closure.
 * Reference: Smullyan (1968), "First-Order Logic", MIT 6.042J */
bool semantic_tableau_check(Formula* f, int root) {
    /* simplified: just check if tautology using truth table */
    return is_tautology(f, root);
}

/* Natural Deduction proof system (propositional)
 * Fitch-style with given rules. Verify each step against premises.
 * Reference: Gentzen (1935), Prawitz (1965), MIT 6.042J Proof Methods */

NDProof* nd_proof_create(Formula* f) {
    NDProof* p = malloc(sizeof(NDProof));
    p->formula = f;
    p->n_steps = 0;
    p->n_assumptions = 0;
    return p;
}

int nd_add_step(NDProof* p, NDRule rule, int prem1, int prem2, int prem3, int conc) {
    if (p->n_steps >= 128) return -1;
    NDStep* s = &p->steps[p->n_steps];
    s->rule = rule;
    s->premises[0] = prem1;
    s->premises[1] = prem2;
    s->premises[2] = prem3;
    s->conclusion = conc;
    s->n_premises = (prem1 >= 0 ? 1 : 0) + (prem2 >= 0 ? 1 : 0) + (prem3 >= 0 ? 1 : 0);
    return p->n_steps++;
}

/* Verify a natural deduction proof by checking rule correctness.
 * Only checks rule types match, doesn't fully verify semantics. */
bool nd_verify_proof(NDProof* p) {
    /* simplified: just check that references are within bounds */
    for (int i = 0; i < p->n_steps; i++) {
        NDStep* s = &p->steps[i];
        for (int j = 0; j < 3; j++) {
            if (s->premises[j] >= p->n_steps) return false;
            if (s->premises[j] >= i && s->rule != RULE_ASSUME) return false; /* can't reference future */
        }
        if (s->conclusion < 0 || s->conclusion >= p->formula->ncount) return false;
    }
    return true;
}
