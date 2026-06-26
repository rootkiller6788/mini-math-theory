#ifndef ABSTRACT_H
#define ABSTRACT_H
#include <stdbool.h>

/* ================================================================
 * Abstract Interpretation (Cousot & Cousot, POPL 1977)
 * L8: Advanced Topics -- Static Program Analysis
 *
 * Framework for sound static analysis using abstract domains.
 * Core concept: Galois connection (α, γ) between concrete domain C
 * and abstract domain A.  α:C->A (abstraction), γ:A->C (concretization).
 *
 * Formal property: ∀c∈C, a∈A. α(c) ⊑ a ⇔ c ⊆ γ(a)
 * where ⊑ is the abstraction ordering and ⊆ is the concrete ordering.
 *
 * This module implements:
 *   1. Sign domain: {⊥, -, 0, +, ⊤} -- tracks sign of values
 *   2. Interval domain: [lo, hi] -- tracks bounds of values
 *   3. Congruence domain: a ≡ b (mod m) -- tracks modular relationships
 *   4. Abstract semantics for integer arithmetic
 *   5. Widening operator (∇) -- ensures termination of fixed-point iteration
 *   6. Narrowing operator (Δ) -- improves precision after widening
 * ================================================================ */

/* --- Sign Abstract Domain --- */
/* Lattice: ⊥ (bottom) ⊑ {-} ⊑ {0} ⊑ {+} ⊑ ⊤ (top)
 *          {-} and {0} are incomparable; {0} and {+} are incomparable */
typedef enum {
    SIGN_BOTTOM = 0,  /* no possible value (unreachable) */
    SIGN_NEG    = 1,  /* strictly negative */
    SIGN_ZERO   = 2,  /* exactly zero */
    SIGN_POS    = 3,  /* strictly positive */
    SIGN_NONNEG = 4,  /* >= 0 */
    SIGN_NONPOS = 5,  /* <= 0 */
    SIGN_NONZERO= 6,  /* != 0 */
    SIGN_TOP    = 7   /* any integer (unknown) */
} Sign;

/* --- Interval Abstract Domain --- */
typedef struct {
    int lo;        /* lower bound */
    int hi;        /* upper bound */
    bool is_bottom; /* empty interval */
} Interval;

/* --- Congruence Abstract Domain --- */
/* Represents: { x | x ≡ residue (mod modulus) }
 * modulus=0 means "any value at all" (top)
 * Negative modulus implies bottom */
typedef struct {
    int modulus;
    int residue;
} Congruence;

/* --- Combined Abstract Value --- */
typedef enum {
    ABSVAL_SIGN,
    ABSVAL_INTERVAL,
    ABSVAL_CONGRUENCE
} AbsValKind;

typedef struct AbsVal {
    AbsValKind kind;
    union {
        Sign       sign;
        Interval   interval;
        Congruence congruence;
    } data;
} AbsVal;

/* --- Abstract State (maps variables to abstract values) --- */
#define ABS_MAX_VARS 32
typedef struct {
    AbsVal vars[ABS_MAX_VARS];
    bool   is_bottom;   /* unreachable state */
    int    nvars;
} AbsState;

/* ================================================================
 * Sign Domain Operations
 * ================================================================ */
const char* sign_name(Sign s);
bool  sign_leq(Sign a, Sign b);         /* a ⊑ b */
Sign  sign_join(Sign a, Sign b);        /* a ⊔ b (least upper bound) */
Sign  sign_meet(Sign a, Sign b);        /* a ⊓ b (greatest lower bound) */
Sign  sign_widen(Sign a, Sign b);       /* a ∇ b */
Sign  sign_narrow(Sign a, Sign b);      /* a Δ b */

/* Abstract transfer functions for sign arithmetic */
Sign  sign_add(Sign a, Sign b);
Sign  sign_sub(Sign a, Sign b);
Sign  sign_mul(Sign a, Sign b);
Sign  sign_div(Sign a, Sign b);
Sign  sign_neg(Sign a);
Sign  sign_abs(Sign a);
Sign  sign_from_int(int val);           /* α({val}) */
bool  sign_contains(Sign s, int val);   /* val ∈ γ(s) */
Sign  sign_filter_eq(Sign s, int val);  /* s ⊓ α({val}) for equality guard */
Sign  sign_filter_ne(Sign s, int val);  /* s ⊓ α(Z\{val}) for != guard */
Sign  sign_filter_lt(Sign s, int val);  /* s restricted by < val */
Sign  sign_filter_le(Sign s, int val);  /* s restricted by <= val */
Sign  sign_filter_gt(Sign s, int val);  /* s restricted by > val */
Sign  sign_filter_ge(Sign s, int val);  /* s restricted by >= val */

/* ================================================================
 * Interval Domain Operations
 * ================================================================ */
Interval interval_make(int lo, int hi);
Interval interval_bottom(void);
Interval interval_top(void);
bool     interval_is_bottom(Interval iv);
bool     interval_leq(Interval a, Interval b);
Interval interval_join(Interval a, Interval b);
Interval interval_meet(Interval a, Interval b);
Interval interval_widen(Interval a, Interval b);
Interval interval_narrow(Interval a, Interval b);

/* Abstract arithmetic on intervals */
Interval interval_add(Interval a, Interval b);
Interval interval_sub(Interval a, Interval b);
Interval interval_mul(Interval a, Interval b);
Interval interval_div(Interval a, Interval b);
Interval interval_neg(Interval a);

/* Filter operations (for conditionals) */
Interval interval_filter_lt(Interval iv, int c);
Interval interval_filter_le(Interval iv, int c);
Interval interval_filter_gt(Interval iv, int c);
Interval interval_filter_ge(Interval iv, int c);
Interval interval_filter_eq(Interval iv, int c);
Interval interval_filter_ne(Interval iv, int c);

bool interval_contains(Interval iv, int val);
void interval_print(Interval iv);

/* ================================================================
 * Congruence Domain Operations
 * ================================================================ */
Congruence congruence_make(int modulus, int residue);
Congruence congruence_top(void);
Congruence congruence_bottom(void);
bool       congruence_leq(Congruence a, Congruence b);
Congruence congruence_join(Congruence a, Congruence b);
Congruence congruence_meet(Congruence a, Congruence b);
bool       congruence_contains(Congruence cg, int val);

/* ================================================================
 * Abstract State Operations
 * ================================================================ */
void      absstate_init(AbsState* st, int nvars);
void      absstate_set_sign(AbsState* st, int vidx, Sign s);
void      absstate_set_interval(AbsState* st, int vidx, Interval iv);
void      absstate_set_congruence(AbsState* st, int vidx, Congruence cg);
AbsVal    absstate_get(AbsState* st, int vidx);
bool      absstate_is_bottom(AbsState* st);
AbsState  absstate_join(AbsState* a, AbsState* b);
AbsState  absstate_widen(AbsState* a, AbsState* b);
bool      absstate_leq(AbsState* a, AbsState* b);
void      absstate_print(AbsState* st);

/* ================================================================
 * Abstract Interpreter for Simple Imperative Language
 * (reuses wp.h expression/statement AST)
 * ================================================================ */
typedef enum {
    ABS_EXPR_CONST, ABS_EXPR_VAR,
    ABS_EXPR_ADD, ABS_EXPR_SUB, ABS_EXPR_MUL, ABS_EXPR_DIV, ABS_EXPR_NEG,
    ABS_EXPR_EQ, ABS_EXPR_NE, ABS_EXPR_LT, ABS_EXPR_LE, ABS_EXPR_GT, ABS_EXPR_GE
} AbsExprKind;

typedef struct AbsExpr {
    AbsExprKind kind;
    int value;
    struct AbsExpr* left;
    struct AbsExpr* right;
} AbsExpr;

typedef enum {
    ABS_STMT_SKIP, ABS_STMT_ASSIGN, ABS_STMT_SEQ,
    ABS_STMT_IF, ABS_STMT_WHILE, ABS_STMT_ASSERT
} AbsStmtKind;

typedef struct AbsStmt {
    AbsStmtKind kind;
    int var_idx;
    AbsExpr* expr;
    AbsExpr* guard;
    struct AbsStmt* left;
    struct AbsStmt* right;
    int loop_id;          /* for while: unique loop identifier */
} AbsStmt;

/* Evaluate abstract expression over abstract state → Interval */
Interval abs_expr_eval_interval(AbsExpr* e, AbsState* st);
Sign      abs_expr_eval_sign(AbsExpr* e, AbsState* st);

/* Transfer function: execute one abstract statement */
AbsState  abs_transfer(AbsStmt* s, AbsState* in_state);

/* Fixed-point iteration (chaotic iteration) for while loops */
AbsState  abs_fixpoint(AbsStmt* loop_body, AbsExpr* guard,
                       int loop_id, AbsState* initial);

/* Full abstract interpretation of a program */
typedef struct {
    AbsState* states;
    int count;
    int capacity;
} AbsTrace;

AbsTrace* abs_interpret(AbsStmt* program, AbsState* initial);
void      abs_trace_free(AbsTrace* t);

/* Constructors */
AbsExpr*  absexpr_const(int v);
AbsExpr*  absexpr_var(int idx);
AbsExpr*  absexpr_binary(AbsExprKind kind, AbsExpr* l, AbsExpr* r);
AbsExpr*  absexpr_unary(AbsExprKind kind, AbsExpr* e);
void      absexpr_free(AbsExpr* e);

AbsStmt*  absstmt_skip(void);
AbsStmt*  absstmt_assign(int idx, AbsExpr* rhs);
AbsStmt*  absstmt_seq(AbsStmt* s1, AbsStmt* s2);
AbsStmt*  absstmt_if(AbsExpr* guard, AbsStmt* t, AbsStmt* e);
AbsStmt*  absstmt_while(AbsExpr* guard, AbsStmt* body, int loop_id);
void      absstmt_free(AbsStmt* s);

#endif
