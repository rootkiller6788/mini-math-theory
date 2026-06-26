#include "wp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================
 * Expression constructors and operations
 * ================================================================ */

Expr* expr_const(int val) {
    Expr* e = malloc(sizeof(Expr));
    e->kind = EXPR_CONST;
    e->value = val;
    e->left = e->right = NULL;
    return e;
}

Expr* expr_var(int idx) {
    Expr* e = malloc(sizeof(Expr));
    e->kind = EXPR_VAR;
    e->value = idx;
    e->left = e->right = NULL;
    return e;
}

Expr* expr_binary(ExprKind kind, Expr* left, Expr* right) {
    Expr* e = malloc(sizeof(Expr));
    e->kind = kind;
    e->value = 0;
    e->left = left;
    e->right = right;
    return e;
}

Expr* expr_unary(ExprKind kind, Expr* e_in) {
    Expr* e = malloc(sizeof(Expr));
    e->kind = kind;
    e->value = 0;
    e->left = e_in;
    e->right = NULL;
    return e;
}

void expr_free(Expr* e) {
    if (!e) return;
    expr_free(e->left);
    expr_free(e->right);
    free(e);
}

Expr* expr_copy(Expr* e) {
    if (!e) return NULL;
    Expr* c = malloc(sizeof(Expr));
    c->kind = e->kind;
    c->value = e->value;
    c->left  = expr_copy(e->left);
    c->right = expr_copy(e->right);
    return c;
}

/* Evaluate expression in a given variable store.
 * Returns 0/1 for boolean expressions. */
int expr_eval(Expr* e, VarStore* vs) {
    if (!e) return 0;
    int l, r;
    switch (e->kind) {
    case EXPR_CONST: return e->value;
    case EXPR_VAR:
        if (e->value >= 0 && e->value < vs->count) return vs->values[e->value];
        fprintf(stderr, "expr_eval: var index %d out of range\n", e->value);
        return 0;
    case EXPR_ADD: return expr_eval(e->left, vs) + expr_eval(e->right, vs);
    case EXPR_SUB: return expr_eval(e->left, vs) - expr_eval(e->right, vs);
    case EXPR_MUL: return expr_eval(e->left, vs) * expr_eval(e->right, vs);
    case EXPR_DIV:
        r = expr_eval(e->right, vs);
        return r != 0 ? expr_eval(e->left, vs) / r : 0;
    case EXPR_NEG: return -expr_eval(e->left, vs);
    case EXPR_EQ: l = expr_eval(e->left, vs); r = expr_eval(e->right, vs); return l == r;
    case EXPR_NE: l = expr_eval(e->left, vs); r = expr_eval(e->right, vs); return l != r;
    case EXPR_LT: l = expr_eval(e->left, vs); r = expr_eval(e->right, vs); return l < r;
    case EXPR_LE: l = expr_eval(e->left, vs); r = expr_eval(e->right, vs); return l <= r;
    case EXPR_GT: l = expr_eval(e->left, vs); r = expr_eval(e->right, vs); return l > r;
    case EXPR_GE: l = expr_eval(e->left, vs); r = expr_eval(e->right, vs); return l >= r;
    case EXPR_AND: return expr_eval(e->left, vs) && expr_eval(e->right, vs);
    case EXPR_OR:  return expr_eval(e->left, vs) || expr_eval(e->right, vs);
    case EXPR_NOT: return !expr_eval(e->left, vs);
    default: return 0;
    }
}

/* Substitute var_idx with replacement throughout expression.
 * Implements Q[x/e] from the wp axiom for assignment. */
Expr* expr_subst(Expr* e, int var_idx, Expr* replacement) {
    if (!e) return NULL;
    if (e->kind == EXPR_VAR && e->value == var_idx) {
        return expr_copy(replacement);
    }
    Expr* r = malloc(sizeof(Expr));
    r->kind = e->kind;
    r->value = e->value;
    r->left  = expr_subst(e->left,  var_idx, replacement);
    r->right = expr_subst(e->right, var_idx, replacement);
    return r;
}

/* Simple tautology check -- only for trivial cases.
 * A full theorem prover would require SAT/SMT solving.
 * Here we recognize: x == x, x <= x, x >= x, NOT(false), etc. */
bool expr_is_tautology(Expr* e) {
    if (!e) return false;
    if (e->kind == EXPR_CONST) return e->value != 0;
    if (e->kind == EXPR_EQ || e->kind == EXPR_LE || e->kind == EXPR_GE) {
        /* Reflexivity: x REL x */
        if (e->left && e->right &&
            e->left->kind == EXPR_VAR && e->right->kind == EXPR_VAR &&
            e->left->value == e->right->value)
            return true;
    }
    if (e->kind == EXPR_AND)
        return expr_is_tautology(e->left) && expr_is_tautology(e->right);
    if (e->kind == EXPR_OR)
        return expr_is_tautology(e->left) || expr_is_tautology(e->right);
    if (e->kind == EXPR_NOT)
        return e->left && e->left->kind == EXPR_CONST && e->left->value == 0;
    return false;
}

/* Render expression as readable string (for debugging).
 * Uses rotating static buffers to handle recursion safely. */
char* expr_to_string(Expr* e) {
    static char bufs[4][1024];
    static int idx = 0;
    idx = (idx + 1) % 4;
    char* buf = bufs[idx];

    if (!e) { snprintf(buf, 1024, "NULL"); return buf; }
    switch (e->kind) {
    case EXPR_CONST: snprintf(buf, 1024, "%d", e->value); break;
    case EXPR_VAR:   snprintf(buf, 1024, "v%d", e->value); break;
    case EXPR_ADD:   { char* l = expr_to_string(e->left);  char* r = expr_to_string(e->right);
                       snprintf(buf, 1024, "(%s+%s)", l, r); } break;
    case EXPR_SUB:   { char* l = expr_to_string(e->left);  char* r = expr_to_string(e->right);
                       snprintf(buf, 1024, "(%s-%s)", l, r); } break;
    case EXPR_MUL:   { char* l = expr_to_string(e->left);  char* r = expr_to_string(e->right);
                       snprintf(buf, 1024, "(%s*%s)", l, r); } break;
    case EXPR_DIV:   { char* l = expr_to_string(e->left);  char* r = expr_to_string(e->right);
                       snprintf(buf, 1024, "(%s/%s)", l, r); } break;
    case EXPR_EQ:    { char* l = expr_to_string(e->left);  char* r = expr_to_string(e->right);
                       snprintf(buf, 1024, "(%s==%s)", l, r); } break;
    case EXPR_NE:    { char* l = expr_to_string(e->left);  char* r = expr_to_string(e->right);
                       snprintf(buf, 1024, "(%s!=%s)", l, r); } break;
    case EXPR_LT:    { char* l = expr_to_string(e->left);  char* r = expr_to_string(e->right);
                       snprintf(buf, 1024, "(%s<%s)", l, r); } break;
    case EXPR_LE:    { char* l = expr_to_string(e->left);  char* r = expr_to_string(e->right);
                       snprintf(buf, 1024, "(%s<=%s)", l, r); } break;
    case EXPR_GT:    { char* l = expr_to_string(e->left);  char* r = expr_to_string(e->right);
                       snprintf(buf, 1024, "(%s>%s)", l, r); } break;
    case EXPR_GE:    { char* l = expr_to_string(e->left);  char* r = expr_to_string(e->right);
                       snprintf(buf, 1024, "(%s>=%s)", l, r); } break;
    case EXPR_AND:   { char* l = expr_to_string(e->left);  char* r = expr_to_string(e->right);
                       snprintf(buf, 1024, "(%s&&%s)", l, r); } break;
    case EXPR_OR:    { char* l = expr_to_string(e->left);  char* r = expr_to_string(e->right);
                       snprintf(buf, 1024, "(%s||%s)", l, r); } break;
    case EXPR_NOT:   snprintf(buf, 1024, "!(%s)", expr_to_string(e->left)); break;
    case EXPR_NEG:   snprintf(buf, 1024, "-(%s)", expr_to_string(e->left)); break;
    default:         snprintf(buf, 1024, "?"); break;
    }
    return buf;
}

/* ================================================================
 * Statement constructors and execution
 * ================================================================ */

Stmt* stmt_skip(void) {
    Stmt* s = malloc(sizeof(Stmt));
    s->kind = STMT_SKIP;
    s->var_index = 0;
    s->expr = s->guard = NULL;
    s->left = s->right = NULL;
    return s;
}

Stmt* stmt_assign(int var_idx, Expr* rhs) {
    Stmt* s = malloc(sizeof(Stmt));
    s->kind = STMT_ASSIGN;
    s->var_index = var_idx;
    s->expr = rhs;
    s->guard = NULL;
    s->left = s->right = NULL;
    return s;
}

Stmt* stmt_seq(Stmt* s1, Stmt* s2) {
    Stmt* s = malloc(sizeof(Stmt));
    s->kind = STMT_SEQ;
    s->var_index = 0;
    s->expr = s->guard = NULL;
    s->left = s1;
    s->right = s2;
    return s;
}

Stmt* stmt_if(Expr* guard, Stmt* then_branch, Stmt* else_branch) {
    Stmt* s = malloc(sizeof(Stmt));
    s->kind = STMT_IF;
    s->var_index = 0;
    s->expr = NULL;
    s->guard = guard;
    s->left = then_branch;
    s->right = else_branch;
    return s;
}

Stmt* stmt_while(Expr* guard, Stmt* body) {
    Stmt* s = malloc(sizeof(Stmt));
    s->kind = STMT_WHILE;
    s->var_index = 0;
    s->expr = NULL;
    s->guard = guard;
    s->left = body;
    s->right = NULL;
    return s;
}

Stmt* stmt_assert(Expr* cond) {
    Stmt* s = malloc(sizeof(Stmt));
    s->kind = STMT_ASSERT;
    s->var_index = 0;
    s->expr = cond;
    s->guard = NULL;
    s->left = s->right = NULL;
    return s;
}

void stmt_free(Stmt* s) {
    if (!s) return;
    expr_free(s->expr);
    if (s->guard) expr_free(s->guard);
    stmt_free(s->left);
    stmt_free(s->right);
    free(s);
}

void stmt_execute(Stmt* s, VarStore* vs) {
    if (!s) return;
    switch (s->kind) {
    case STMT_SKIP:
        break;
    case STMT_ASSIGN:
        if (s->var_index >= 0 && s->var_index < vs->count)
            vs->values[s->var_index] = expr_eval(s->expr, vs);
        break;
    case STMT_SEQ:
        stmt_execute(s->left, vs);
        stmt_execute(s->right, vs);
        break;
    case STMT_IF:
        if (expr_eval(s->guard, vs))
            stmt_execute(s->left, vs);
        else
            stmt_execute(s->right, vs);
        break;
    case STMT_WHILE:
        while (expr_eval(s->guard, vs))
            stmt_execute(s->left, vs);
        break;
    case STMT_ASSERT:
        if (!expr_eval(s->expr, vs)) {
            fprintf(stderr, "ASSERT FAILED: %s\n", expr_to_string(s->expr));
            abort();
        }
        break;
    default: break;
    }
}

/* ================================================================
 * Dijkstra's Weakest Precondition Computation
 *
 * wp(skip, Q)               = Q
 * wp(x := e, Q)             = Q[x/e]
 * wp(S1 ; S2, Q)            = wp(S1, wp(S2, Q))
 * wp(if B then S1 else S2, Q) = (B => wp(S1, Q)) /\ (~B => wp(S2, Q))
 * wp(while B inv I do S, Q) = I
 *   /\ (I /\ B => wp(S, I))
 *   /\ (I /\ ~B => Q)
 *
 * Note: For while loops, the user must provide an invariant I.
 * Without one, we return a conservative approximation (false).
 * ================================================================ */

Expr* wp(Stmt* S, Expr* Q) {
    if (!S) return expr_copy(Q);
    switch (S->kind) {
    case STMT_SKIP:
        return expr_copy(Q);

    case STMT_ASSIGN:
        /* wp(x:=e, Q) = Q[x/e] */
        return expr_subst(Q, S->var_index, S->expr);

    case STMT_SEQ:
        /* wp(S1;S2, Q) = wp(S1, wp(S2, Q)) */
        {
            Expr* wp_s2 = wp(S->right, Q);
            Expr* result = wp(S->left, wp_s2);
            expr_free(wp_s2);
            return result;
        }

    case STMT_IF: {
        /* wp(if B then S1 else S2, Q)
         *   = (B && wp(S1, Q)) || (!B && wp(S2, Q)) */
        Expr* wp_then = wp(S->left, Q);
        Expr* wp_else = wp(S->right, Q);
        Expr* guard_true  = expr_copy(S->guard);
        Expr* guard_false = expr_unary(EXPR_NOT, expr_copy(S->guard));
        Expr* cond_then = expr_binary(EXPR_AND, guard_true, wp_then);
        Expr* cond_else = expr_binary(EXPR_AND, guard_false, wp_else);
        return expr_binary(EXPR_OR, cond_then, cond_else);
    }

    case STMT_WHILE:
        /* Without an explicit invariant, we cannot compute wp.
         * Return false marker. Use vcgen_generate with invariants. */
        return expr_const(0);

    case STMT_ASSERT:
        /* wp(assert(R), Q) = R /\ Q */
        return expr_binary(EXPR_AND, expr_copy(S->expr), expr_copy(Q));

    default:
        return expr_copy(Q);
    }
}

/* ================================================================
 * Strongest Postcondition (dual of wp)
 *
 * sp(P, skip)             = P
 * sp(P, x:=e)             = x == e /\ P  (approx, sound when x not in e)
 * sp(P, S1;S2)            = sp(sp(P, S1), S2)
 * sp(P, if B then S1 else S2) = sp(P/\B, S1) \/ sp(P/\~B, S2)
 * ================================================================ */

Expr* sp(Stmt* S, Expr* P) {
    if (!S) return expr_copy(P);
    switch (S->kind) {
    case STMT_SKIP:
        return expr_copy(P);

    case STMT_ASSIGN: {
        /* sp(P, x:=e) = x == e /\ P (sound over-approximation) */
        Expr* eq = expr_binary(EXPR_EQ,
            expr_var(S->var_index),
            expr_copy(S->expr));
        return expr_binary(EXPR_AND, expr_copy(P), eq);
    }

    case STMT_SEQ: {
        Expr* sp_s1 = sp(S->left, P);
        Expr* result = sp(S->right, sp_s1);
        expr_free(sp_s1);
        return result;
    }

    case STMT_IF: {
        Expr* guard_true  = expr_copy(S->guard);
        Expr* guard_false = expr_unary(EXPR_NOT, expr_copy(S->guard));
        Expr* p_and_g     = expr_binary(EXPR_AND, expr_copy(P), guard_true);
        Expr* p_and_not_g = expr_binary(EXPR_AND, expr_copy(P), guard_false);
        Expr* sp_then = sp(S->left,  p_and_g);
        Expr* sp_else = sp(S->right, p_and_not_g);
        expr_free(p_and_g);
        expr_free(p_and_not_g);
        return expr_binary(EXPR_OR, sp_then, sp_else);
    }

    case STMT_WHILE:
        return expr_const(0);

    case STMT_ASSERT:
        return expr_binary(EXPR_AND, expr_copy(P), expr_copy(S->expr));

    default:
        return expr_copy(P);
    }
}

/* ================================================================
 * Verification Condition Generator
 * Based on Floyd-Hoare logic.
 * ================================================================ */

VCGen* vcgen_create(void) {
    VCGen* vcg = malloc(sizeof(VCGen));
    vcg->capacity = 64;
    vcg->count = 0;
    vcg->conditions = malloc(vcg->capacity * sizeof(Expr*));
    return vcg;
}

void vcgen_add(VCGen* vcg, Expr* cond) {
    if (vcg->count >= vcg->capacity) {
        vcg->capacity *= 2;
        vcg->conditions = realloc(vcg->conditions, vcg->capacity * sizeof(Expr*));
    }
    vcg->conditions[vcg->count++] = cond;
}

/* Generate wp-based VC: P => wp(S, Q) */
void vcgen_generate(VCGen* vcg, Stmt* S, Expr* pre, Expr* post) {
    Expr* wp_sq = wp(S, post);
    Expr* not_pre = expr_unary(EXPR_NOT, expr_copy(pre));
    Expr* impl = expr_binary(EXPR_OR, not_pre, wp_sq);
    vcgen_add(vcg, impl);
}

bool vcgen_verify(VCGen* vcg, VarStore* vs) {
    printf("=== Verification Conditions (%d total) ===\n", vcg->count);
    bool all_pass = true;
    for (int i = 0; i < vcg->count; i++) {
        int val = expr_eval(vcg->conditions[i], vs);
        printf("  VC[%d]: %s = %s\n", i, expr_to_string(vcg->conditions[i]),
               val ? "VALID" : "INVALID");
        if (!val) all_pass = false;
    }
    printf("  Result: %s\n\n", all_pass ? "ALL VALID" : "SOME INVALID");
    return all_pass;
}

void vcgen_free(VCGen* vcg) {
    if (!vcg) return;
    for (int i = 0; i < vcg->count; i++)
        expr_free(vcg->conditions[i]);
    free(vcg->conditions);
    free(vcg);
}

/* Hoare triple check: {P} S {Q} evaluated in given state */
bool hoare_triple_check(Stmt* S, Expr* P, Expr* Q, VarStore* vs) {
    int saved[MAX_VARS];
    for (int i = 0; i < vs->count; i++)
        saved[i] = vs->values[i];

    if (!expr_eval(P, vs)) {
        /* {P} S {Q} vacuously true when P is false */
        return true;
    }

    stmt_execute(S, vs);
    bool ok = expr_eval(Q, vs);

    for (int i = 0; i < vs->count; i++)
        vs->values[i] = saved[i];

    return ok;
}

/* ================================================================
 * Variable Store
 * ================================================================ */

void varstore_init(VarStore* vs) {
    vs->count = 0;
    memset(vs->values, 0, sizeof(vs->values));
    memset(vs->names, 0, sizeof(vs->names));
}

int varstore_add(VarStore* vs, const char* name, int value) {
    if (vs->count >= MAX_VARS) return -1;
    vs->names[vs->count] = _strdup(name);
    vs->values[vs->count] = value;
    return vs->count++;
}

void varstore_set(VarStore* vs, int idx, int value) {
    if (idx >= 0 && idx < vs->count)
        vs->values[idx] = value;
}

int varstore_get(VarStore* vs, int idx) {
    if (idx >= 0 && idx < vs->count)
        return vs->values[idx];
    return 0;
}
