#ifndef WP_H
#define WP_H
#include <stdbool.h>

/* ================================================================
 * Weakest Precondition Calculus (Dijkstra 1975)
 * L4: Standards/Theorems -- Hoare Logic / Predicate Transformer Semantics
 *
 * Reference: Dijkstra, "Guarded Commands, Nondeterminacy and Formal
 * Derivation of Programs", CACM 1975.
 *
 * wp(S, Q) = weakest precondition such that executing S in a state
 *            satisfying wp(S, Q) guarantees termination in a state
 *            satisfying Q.
 *
 * Axioms:
 *   wp(skip, Q)     = Q
 *   wp(x := e, Q)   = Q[x/e]
 *   wp(S1;S2, Q)    = wp(S1, wp(S2, Q))
 *   wp(if B->S1 fi, Q) = (B => wp(S1, Q))
 *   wp(do B->S1 od, Q) = exists k. H_k(Q)
 *     where H_0(Q) = !B && Q, H_{k+1} = H_0 || wp(if B->S1 fi, H_k)
 * ================================================================ */

/* --- Expression Language (integer arithmetic) --- */
typedef enum {
    EXPR_CONST,          /* integer literal */
    EXPR_VAR,            /* variable reference by name index */
    EXPR_ADD, EXPR_SUB, EXPR_MUL, EXPR_DIV,
    EXPR_EQ, EXPR_NE, EXPR_LT, EXPR_LE, EXPR_GT, EXPR_GE,
    EXPR_AND, EXPR_OR, EXPR_NOT,
    EXPR_NEG
} ExprKind;

typedef struct Expr {
    ExprKind kind;
    int value;           /* for CONST or VAR index */
    struct Expr* left;   /* binary op left operand */
    struct Expr* right;  /* binary op right operand */
} Expr;

/* --- Simple Imperative Language --- */
typedef enum {
    STMT_SKIP,
    STMT_ASSIGN,         /* x := e */
    STMT_SEQ,            /* S1 ; S2 */
    STMT_IF,             /* if B then S1 else S2 */
    STMT_WHILE,          /* while B do S */
    STMT_ASSERT          /* assert(Q) */
} StmtKind;

typedef struct Stmt {
    StmtKind kind;
    int var_index;           /* for ASSIGN: target variable */
    struct Expr* expr;       /* for ASSIGN: rhs expression */
    struct Expr* guard;      /* for IF/WHILE: boolean condition */
    struct Stmt* left;       /* for SEQ/IF/WHILE: first branch / body */
    struct Stmt* right;      /* for SEQ/IF: second branch */
} Stmt;

/* --- Variable Store --- */
#define MAX_VARS 32
typedef struct {
    int values[MAX_VARS];
    int count;
    char* names[MAX_VARS];
} VarStore;

/* --- Core API --- */
Expr*   expr_const(int val);
Expr*   expr_var(int idx);
Expr*   expr_binary(ExprKind kind, Expr* left, Expr* right);
Expr*   expr_unary(ExprKind kind, Expr* e);
void    expr_free(Expr* e);
Expr*   expr_copy(Expr* e);
int     expr_eval(Expr* e, VarStore* vs);
Expr*   expr_subst(Expr* e, int var_idx, Expr* replacement);
char*   expr_to_string(Expr* e);
bool    expr_is_tautology(Expr* e);

Stmt*   stmt_skip(void);
Stmt*   stmt_assign(int var_idx, Expr* rhs);
Stmt*   stmt_seq(Stmt* s1, Stmt* s2);
Stmt*   stmt_if(Expr* guard, Stmt* then_branch, Stmt* else_branch);
Stmt*   stmt_while(Expr* guard, Stmt* body);
Stmt*   stmt_assert(Expr* cond);
void    stmt_free(Stmt* s);
void    stmt_execute(Stmt* s, VarStore* vs);

/* Dijkstra's Weakest Precondition */
Expr*   wp(Stmt* S, Expr* Q);

/* Strongest Postcondition (dual of wp) */
Expr*   sp(Stmt* S, Expr* P);

/* Verification condition generator */
typedef struct {
    Expr** conditions;
    int    count;
    int    capacity;
} VCGen;

VCGen*  vcgen_create(void);
void    vcgen_add(VCGen* vcg, Expr* cond);
void    vcgen_generate(VCGen* vcg, Stmt* S, Expr* pre, Expr* post);
bool    vcgen_verify(VCGen* vcg, VarStore* vs);
void    vcgen_free(VCGen* vcg);

/* Hoare Triple: {P} S {Q} */
bool    hoare_triple_check(Stmt* S, Expr* P, Expr* Q, VarStore* vs);

void    varstore_init(VarStore* vs);
int     varstore_add(VarStore* vs, const char* name, int value);
void    varstore_set(VarStore* vs, int idx, int value);
int     varstore_get(VarStore* vs, int idx);

#endif
