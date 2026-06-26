#include "concolic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ================================================================
 * Symbolic Expression Operations
 * ================================================================ */

SymExpr* sym_expr_const(int val) {
    SymExpr* e = malloc(sizeof(SymExpr));
    e->kind = SEXPR_CONST;
    e->value = val;
    e->left = e->right = NULL;
    return e;
}

SymExpr* sym_expr_var(int idx) {
    SymExpr* e = malloc(sizeof(SymExpr));
    e->kind = SEXPR_SYM;
    e->value = idx;
    e->left = e->right = NULL;
    return e;
}

SymExpr* sym_expr_binary(SymExprKind kind, SymExpr* l, SymExpr* r) {
    SymExpr* e = malloc(sizeof(SymExpr));
    e->kind = kind;
    e->value = 0;
    e->left = l;
    e->right = r;
    return e;
}

SymExpr* sym_expr_unary(SymExprKind kind, SymExpr* e_in) {
    SymExpr* e = malloc(sizeof(SymExpr));
    e->kind = kind;
    e->value = 0;
    e->left = e_in;
    e->right = NULL;
    return e;
}

void sym_expr_free(SymExpr* e) {
    if (!e) return;
    sym_expr_free(e->left);
    sym_expr_free(e->right);
    free(e);
}

SymExpr* sym_expr_copy(SymExpr* e) {
    if (!e) return NULL;
    SymExpr* c = malloc(sizeof(SymExpr));
    c->kind = e->kind;
    c->value = e->value;
    c->left  = sym_expr_copy(e->left);
    c->right = sym_expr_copy(e->right);
    return c;
}

/* Evaluate symbolic expression with concrete values for variables */
int sym_expr_eval(SymExpr* e, int* concrete_vals) {
    if (!e) return 0;
    int l, r;
    switch (e->kind) {
    case SEXPR_CONST: return e->value;
    case SEXPR_SYM:   return concrete_vals[e->value];
    case SEXPR_ADD:   return sym_expr_eval(e->left, concrete_vals) + sym_expr_eval(e->right, concrete_vals);
    case SEXPR_SUB:   return sym_expr_eval(e->left, concrete_vals) - sym_expr_eval(e->right, concrete_vals);
    case SEXPR_MUL:   return sym_expr_eval(e->left, concrete_vals) * sym_expr_eval(e->right, concrete_vals);
    case SEXPR_DIV:
        r = sym_expr_eval(e->right, concrete_vals);
        return r != 0 ? sym_expr_eval(e->left, concrete_vals) / r : 0;
    case SEXPR_MOD:
        r = sym_expr_eval(e->right, concrete_vals);
        return r != 0 ? sym_expr_eval(e->left, concrete_vals) % r : 0;
    case SEXPR_EQ:    l = sym_expr_eval(e->left, concrete_vals); r = sym_expr_eval(e->right, concrete_vals); return l == r;
    case SEXPR_NE:    l = sym_expr_eval(e->left, concrete_vals); r = sym_expr_eval(e->right, concrete_vals); return l != r;
    case SEXPR_LT:    l = sym_expr_eval(e->left, concrete_vals); r = sym_expr_eval(e->right, concrete_vals); return l < r;
    case SEXPR_LE:    l = sym_expr_eval(e->left, concrete_vals); r = sym_expr_eval(e->right, concrete_vals); return l <= r;
    case SEXPR_GT:    l = sym_expr_eval(e->left, concrete_vals); r = sym_expr_eval(e->right, concrete_vals); return l > r;
    case SEXPR_GE:    l = sym_expr_eval(e->left, concrete_vals); r = sym_expr_eval(e->right, concrete_vals); return l >= r;
    case SEXPR_AND:   return sym_expr_eval(e->left, concrete_vals) && sym_expr_eval(e->right, concrete_vals);
    case SEXPR_OR:    return sym_expr_eval(e->left, concrete_vals) || sym_expr_eval(e->right, concrete_vals);
    case SEXPR_NOT:   return !sym_expr_eval(e->left, concrete_vals);
    case SEXPR_BITAND:return sym_expr_eval(e->left, concrete_vals) & sym_expr_eval(e->right, concrete_vals);
    case SEXPR_BITOR: return sym_expr_eval(e->left, concrete_vals) | sym_expr_eval(e->right, concrete_vals);
    case SEXPR_BITXOR:return sym_expr_eval(e->left, concrete_vals) ^ sym_expr_eval(e->right, concrete_vals);
    case SEXPR_BITNOT:return ~sym_expr_eval(e->left, concrete_vals);
    case SEXPR_SHL:   return sym_expr_eval(e->left, concrete_vals) << sym_expr_eval(e->right, concrete_vals);
    case SEXPR_SHR:   return sym_expr_eval(e->left, concrete_vals) >> sym_expr_eval(e->right, concrete_vals);
    default: return 0;
    }
}

/* Simplify expression using constant folding */
SymExpr* sym_expr_simplify(SymExpr* e) {
    if (!e) return NULL;
    /* Simplify children first */
    SymExpr* l = e->left  ? sym_expr_simplify(e->left)  : NULL;
    SymExpr* r = e->right ? sym_expr_simplify(e->right) : NULL;

    /* If both children are constants, evaluate directly */
    if (l && r && l->kind == SEXPR_CONST && r->kind == SEXPR_CONST) {
        int fake_vals[1] = {0};
        int result = sym_expr_eval(e, fake_vals);
        sym_expr_free(l);
        sym_expr_free(r);
        return sym_expr_const(result);
    }

    if (e->kind == SEXPR_CONST || e->kind == SEXPR_SYM) {
        sym_expr_free(l); sym_expr_free(r);
        return sym_expr_copy(e);
    }

    SymExpr* result = malloc(sizeof(SymExpr));
    result->kind = e->kind;
    result->value = e->value;
    result->left = l;
    result->right = r;
    return result;
}

char* sym_expr_to_string(SymExpr* e) {
    static char bufs[4][512];
    static int idx = 0;
    idx = (idx + 1) % 4;
    char* buf = bufs[idx];

    if (!e) { snprintf(buf, 512, "NULL"); return buf; }
    switch (e->kind) {
    case SEXPR_CONST: snprintf(buf, 512, "%d", e->value); break;
    case SEXPR_SYM:   snprintf(buf, 512, "x%d", e->value); break;
    case SEXPR_ADD:   { char* l=sym_expr_to_string(e->left); char* r=sym_expr_to_string(e->right);
                         snprintf(buf,512,"(%s+%s)",l,r); } break;
    case SEXPR_SUB:   { char* l=sym_expr_to_string(e->left); char* r=sym_expr_to_string(e->right);
                         snprintf(buf,512,"(%s-%s)",l,r); } break;
    case SEXPR_MUL:   { char* l=sym_expr_to_string(e->left); char* r=sym_expr_to_string(e->right);
                         snprintf(buf,512,"(%s*%s)",l,r); } break;
    case SEXPR_DIV:   { char* l=sym_expr_to_string(e->left); char* r=sym_expr_to_string(e->right);
                         snprintf(buf,512,"(%s/%s)",l,r); } break;
    case SEXPR_MOD:   { char* l=sym_expr_to_string(e->left); char* r=sym_expr_to_string(e->right);
                         snprintf(buf,512,"(%s mod %s)",l,r); } break;
    case SEXPR_EQ:    { char* l=sym_expr_to_string(e->left); char* r=sym_expr_to_string(e->right);
                         snprintf(buf,512,"(%s==%s)",l,r); } break;
    case SEXPR_NE:    { char* l=sym_expr_to_string(e->left); char* r=sym_expr_to_string(e->right);
                         snprintf(buf,512,"(%s!=%s)",l,r); } break;
    case SEXPR_LT:    { char* l=sym_expr_to_string(e->left); char* r=sym_expr_to_string(e->right);
                         snprintf(buf,512,"(%s<%s)",l,r); } break;
    case SEXPR_LE:    { char* l=sym_expr_to_string(e->left); char* r=sym_expr_to_string(e->right);
                         snprintf(buf,512,"(%s<=%s)",l,r); } break;
    case SEXPR_GT:    { char* l=sym_expr_to_string(e->left); char* r=sym_expr_to_string(e->right);
                         snprintf(buf,512,"(%s>%s)",l,r); } break;
    case SEXPR_GE:    { char* l=sym_expr_to_string(e->left); char* r=sym_expr_to_string(e->right);
                         snprintf(buf,512,"(%s>=%s)",l,r); } break;
    default: snprintf(buf, 512, "?"); break;
    }
    return buf;
}

/* ================================================================
 * Constraint Checking and Solving
 * ================================================================ */

bool check_constraint_satisfied(PathConstraint* c, int* values) {
    int lhs_val = sym_expr_eval(c->lhs, values);
    int rhs_val = c->rhs ? sym_expr_eval(c->rhs, values) : 0;
    switch (c->kind) {
    case PC_EQ: return lhs_val == rhs_val;
    case PC_NE: return lhs_val != rhs_val;
    case PC_LT: return lhs_val < rhs_val;
    case PC_LE: return lhs_val <= rhs_val;
    case PC_GT: return lhs_val > rhs_val;
    case PC_GE: return lhs_val >= rhs_val;
    default: return false;
    }
}

bool concolic_check_constraint(PathConstraint* pc, int* concrete) {
    return check_constraint_satisfied(pc, concrete);
}

/* Simple linear constraint solver.
 * For constraint of form: x_i + c1 < c2 (where x_i is symbolic var)
 * Solution: x_i = solve for satisfying value.
 *
 * This is a minimal solver adequate for simple path conditions.
 * Real concolic tools use Z3/STP/Yices for full SMT solving. */
int solve_linear_eq(SymExpr* lhs, int rhs, int* values) {
    (void)values;
    /* Handle: x_i + const OP rhs */
    if (lhs->kind == SEXPR_ADD) {
        if (lhs->left->kind == SEXPR_SYM && lhs->right->kind == SEXPR_CONST) {
            int const_term = lhs->right->value;
            return rhs - const_term;  /* x + c = rhs => x = rhs - c */
        }
        if (lhs->right->kind == SEXPR_SYM && lhs->left->kind == SEXPR_CONST) {
            int const_term = lhs->left->value;
            return rhs - const_term;
        }
    }
    /* Handle: x_i OP rhs */
    if (lhs->kind == SEXPR_SYM) {
        return rhs;  /* x = rhs */
    }
    /* Handle: x_i - const OP rhs */
    if (lhs->kind == SEXPR_SUB) {
        if (lhs->left->kind == SEXPR_SYM && lhs->right->kind == SEXPR_CONST) {
            return rhs + lhs->right->value;  /* x - c = rhs => x = rhs + c */
        }
    }
    /* Conservative: return whatever's there */
    if (lhs->kind == SEXPR_SYM) return rhs;
    /* Fallback: try to solve with current values, return as-is */
    return sym_expr_eval(lhs, values);
}

/* Solve a system of path constraints.
 * Strategy: iteratively adjust inputs to satisfy each constraint.
 * This is a simple greedy solver — not complete but practical. */
bool solve_constraint_system(PathCondition* pc, int* inputs, int nvars) {
    (void)nvars;
    int max_attempts = 1000;
    for (int attempt = 0; attempt < max_attempts; attempt++) {
        bool all_satisfied = true;
        for (int i = 0; i < pc->count; i++) {
            if (!check_constraint_satisfied(&pc->constraints[i], inputs)) {
                all_satisfied = false;
                /* Try to adjust the variable involved */
                SymExpr* lhs = pc->constraints[i].lhs;
                int rhs_val = pc->constraints[i].rhs ?
                    sym_expr_eval(pc->constraints[i].rhs, inputs) : 0;

                /* Find the symbolic variable in LHS and adjust */
                if (lhs->kind == SEXPR_SYM) {
                    int var = lhs->value;
                    switch (pc->constraints[i].kind) {
                    case PC_EQ: inputs[var] = rhs_val; break;
                    case PC_NE: inputs[var] = rhs_val + 1; break;
                    case PC_LT: inputs[var] = rhs_val - 1; break;
                    case PC_LE: inputs[var] = rhs_val; break;
                    case PC_GT: inputs[var] = rhs_val + 1; break;
                    case PC_GE: inputs[var] = rhs_val; break;
                    }
                } else if (lhs->kind == SEXPR_ADD) {
                    if (lhs->left->kind == SEXPR_SYM && lhs->right->kind == SEXPR_CONST) {
                        int var = lhs->left->value;
                        int c = lhs->right->value;
                        switch (pc->constraints[i].kind) {
                        case PC_EQ: inputs[var] = rhs_val - c; break;
                        case PC_NE: inputs[var] = rhs_val - c + 1; break;
                        case PC_LT: inputs[var] = rhs_val - c - 1; break;
                        case PC_LE: inputs[var] = rhs_val - c; break;
                        case PC_GT: inputs[var] = rhs_val - c + 1; break;
                        case PC_GE: inputs[var] = rhs_val - c; break;
                        }
                    }
                }
            }
        }
        if (all_satisfied) return true;
    }
    return false;
}

bool concolic_solve(PathCondition* pc, int* solution, int nvars) {
    return solve_constraint_system(pc, solution, nvars);
}

/* ================================================================
 * Concolic Engine
 * ================================================================ */

void concolic_init(ConcolicEngine* eng, ExploreStrategy strategy, int max_iter) {
    eng->state_capacity = 256;
    eng->states = malloc(eng->state_capacity * sizeof(ConState));
    eng->state_count = 0;
    eng->strategy = strategy;
    eng->total_paths = 0;
    eng->covered_branches = 0;
    eng->max_iterations = max_iter;
    srand((unsigned int)time(NULL));
}

ConState* concolic_create_state(ConcolicEngine* eng, int nvars, int* initial_values) {
    if (eng->state_count >= eng->state_capacity) {
        eng->state_capacity *= 2;
        eng->states = realloc(eng->states, eng->state_capacity * sizeof(ConState));
    }
    ConState* st = &eng->states[eng->state_count++];
    st->nvars = (nvars < MAX_SYM_VARS_CON) ? nvars : MAX_SYM_VARS_CON;
    st->path_id = eng->total_paths++;
    st->terminated = false;
    st->error_code = 0;
    st->pc.count = 0;
    st->pc.next_to_negate = 0;
    memcpy(st->concrete, initial_values, st->nvars * sizeof(int));
    return st;
}

void concolic_add_constraint(ConState* st, PCKind kind, SymExpr* lhs,
                              int rhs, bool taken, int branch_id) {
    if (st->pc.count >= MAX_CONSTRAINTS) return;
    PathConstraint* pc = &st->pc.constraints[st->pc.count++];
    pc->kind = kind;
    pc->lhs = lhs;
    pc->rhs = sym_expr_const(rhs);
    pc->taken = taken;
    pc->branch_id = branch_id;
}

/* Generate next input by negating one constraint and solving */
bool concolic_generate_next(ConcolicEngine* eng, ConState* parent,
                             int* new_inputs) {
    if (parent->pc.count == 0) return false;

    /* Build new path condition by negating one constraint */
    int negate_idx;
    switch (eng->strategy) {
    case EXPLORE_DFS:
        /* DFS: negate the last un-negated constraint */
        negate_idx = parent->pc.count - 1;
        while (negate_idx >= 0 && !parent->pc.constraints[negate_idx].taken)
            negate_idx--;
        if (negate_idx < 0) return false;
        break;
    case EXPLORE_BFS:
        /* BFS: negate the first constraint */
        negate_idx = parent->pc.next_to_negate;
        if (negate_idx >= parent->pc.count) return false;
        break;
    case EXPLORE_RANDOM:
        negate_idx = rand() % parent->pc.count;
        break;
    default:
        negate_idx = parent->pc.count - 1;
    }

    /* Copy first negate_idx constraints */
    PathCondition new_pc;
    new_pc.count = negate_idx + 1;
    for (int i = 0; i < negate_idx; i++) {
        new_pc.constraints[i] = parent->pc.constraints[i];
    }
    /* Negate the chosen constraint */
    new_pc.constraints[negate_idx] = parent->pc.constraints[negate_idx];
    new_pc.constraints[negate_idx].taken = false;
    /* Invert the comparison */
    switch (new_pc.constraints[negate_idx].kind) {
    case PC_EQ: new_pc.constraints[negate_idx].kind = PC_NE; break;
    case PC_NE: new_pc.constraints[negate_idx].kind = PC_EQ; break;
    case PC_LT: new_pc.constraints[negate_idx].kind = PC_GE; break;
    case PC_LE: new_pc.constraints[negate_idx].kind = PC_GT; break;
    case PC_GT: new_pc.constraints[negate_idx].kind = PC_LE; break;
    case PC_GE: new_pc.constraints[negate_idx].kind = PC_LT; break;
    }

    /* Try to solve */
    memcpy(new_inputs, parent->concrete, parent->nvars * sizeof(int));
    return solve_constraint_system(&new_pc, new_inputs, parent->nvars);
}

/* Concolic exploration driver */
void concolic_explore(ConcolicEngine* eng,
                       ConcolicProgram program, void* user_data,
                       int nvars, int* initial_inputs) {
    printf("===== Concolic Exploration =====\n");
    printf("Strategy: %d, Max iterations: %d, Vars: %d\n",
           eng->strategy, eng->max_iterations, nvars);

    for (int iter = 0; iter < eng->max_iterations; iter++) {
        /* Create state with current inputs */
        ConState* st = concolic_create_state(eng, nvars, initial_inputs);

        /* Execute program concretely, collecting path constraints */
        program(st, user_data);

        eng->covered_branches = st->pc.count;

        printf("  Path %d: inputs=[", st->path_id);
        for (int i = 0; i < nvars; i++)
            printf("%s%d", i > 0 ? "," : "", st->concrete[i]);
        printf("], branches=%d", st->pc.count);
        if (st->error_code != 0)
            printf(", ERROR=%d", st->error_code);
        printf("\n");

        /* Try to generate next inputs by negating a constraint */
        int new_inputs[MAX_SYM_VARS_CON];
        if (!concolic_generate_next(eng, st, new_inputs))
            break;

        memcpy(initial_inputs, new_inputs, nvars * sizeof(int));
    }

    concolic_report(eng);
}

void concolic_report(ConcolicEngine* eng) {
    printf("\n--- Concolic Testing Report ---\n");
    printf("Total paths explored: %d\n", eng->total_paths);
    printf("States recorded:      %d\n", eng->state_count);
    printf("Branches covered:     %d\n", eng->covered_branches);
    printf("Errors found:         ");
    int errors = 0;
    for (int i = 0; i < eng->state_count; i++) {
        if (eng->states[i].error_code != 0) {
            errors++;
            printf("state[%d](code=%d) ", i, eng->states[i].error_code);
        }
    }
    if (errors == 0) printf("none");
    printf("\n");
}

void concolic_free(ConcolicEngine* eng) {
    /* Free symbolic expressions in constraints */
    for (int i = 0; i < eng->state_count; i++) {
        for (int j = 0; j < eng->states[i].pc.count; j++) {
            sym_expr_free(eng->states[i].pc.constraints[j].lhs);
            sym_expr_free(eng->states[i].pc.constraints[j].rhs);
        }
    }
    free(eng->states);
    eng->states = NULL;
    eng->state_count = 0;
}
