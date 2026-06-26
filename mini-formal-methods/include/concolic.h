#ifndef CONCOLIC_H
#define CONCOLIC_H
#include <stdbool.h>

/* ================================================================
 * Concolic Testing (Concrete + Symbolic Execution)
 * L8: Advanced Topics -- Dynamic Symbolic Execution
 *
 * Based on: Godefroid, Klarlund, Sen (2005)
 * "DART: Directed Automated Random Testing"
 * and Cadar, Dunbar, Engler (2008) "KLEE: Unassisted and Automatic
 * Generation of High-Coverage Tests for Complex Systems Programs"
 *
 * Concolic = CONCrete + symbOLIC execution.
 *
 * The program is executed with concrete inputs while simultaneously
 * collecting symbolic path constraints. When a path is explored,
 * the next path is chosen by negating one constraint and solving
 * the resulting constraint system.
 *
 * This module implements:
 *   1. Symbolic variables and expressions
 *   2. Path constraint collection
 *   3. Constraint solving (simple linear integer constraints)
 *   4. Path exploration strategies (BFS, DFS, random)
 *   5. Coverage-guided test generation
 * ================================================================ */

/* --- Symbolic Expression --- */
typedef enum {
    SEXPR_CONST,    /* integer constant */
    SEXPR_SYM,      /* symbolic variable */
    SEXPR_ADD, SEXPR_SUB, SEXPR_MUL, SEXPR_DIV, SEXPR_MOD,
    SEXPR_EQ, SEXPR_NE, SEXPR_LT, SEXPR_LE, SEXPR_GT, SEXPR_GE,
    SEXPR_AND, SEXPR_OR, SEXPR_NOT,
    SEXPR_SHL, SEXPR_SHR,  /* bitwise shifts */
    SEXPR_BITAND, SEXPR_BITOR, SEXPR_BITXOR, SEXPR_BITNOT
} SymExprKind;

typedef struct SymExpr {
    SymExprKind kind;
    int value;
    struct SymExpr* left;
    struct SymExpr* right;
} SymExpr;

/* --- Path Constraint --- */
typedef enum {
    PC_EQ, PC_NE, PC_LT, PC_LE, PC_GT, PC_GE
} PCKind;

typedef struct PathConstraint {
    PCKind kind;       /* comparison operator */
    SymExpr* lhs;      /* left-hand expression */
    SymExpr* rhs;      /* right-hand expression (always CONST for simplicity) */
    bool taken;        /* true if branch was taken, for negation */
    int branch_id;     /* unique branch identifier */
} PathConstraint;

/* --- Path Condition (set of constraints) --- */
#define MAX_CONSTRAINTS 128
typedef struct PathCondition {
    PathConstraint constraints[MAX_CONSTRAINTS];
    int count;
    int next_to_negate;  /* for systematic path exploration */
} PathCondition;

/* --- Concolic State --- */
#define MAX_SYM_VARS_CON 16
typedef struct ConState {
    int concrete[MAX_SYM_VARS_CON];     /* concrete values */
    int nvars;
    PathCondition pc;                   /* path constraints collected so far */
    int path_id;
    bool terminated;                    /* normal or error termination */
    int error_code;                     /* 0 = normal, non-zero = error type */
} ConState;

/* --- Exploration Strategy --- */
typedef enum {
    EXPLORE_DFS,          /* depth-first: negate last constraint */
    EXPLORE_BFS,          /* breadth-first: negate earliest constraint */
    EXPLORE_RANDOM,       /* random constraint negation */
    EXPLORE_COVERAGE,     /* coverage-guided */
    EXPLORE_GENERATIONAL  /* generational search */
} ExploreStrategy;

/* --- Concolic Engine --- */
typedef struct ConcolicEngine {
    ConState* states;
    int state_count;
    int state_capacity;
    ExploreStrategy strategy;
    int total_paths;
    int covered_branches;
    int max_iterations;
} ConcolicEngine;

/* ================================================================
 * Symbolic Expression API
 * ================================================================ */
SymExpr* sym_expr_const(int val);
SymExpr* sym_expr_var(int idx);
SymExpr* sym_expr_binary(SymExprKind kind, SymExpr* l, SymExpr* r);
SymExpr* sym_expr_unary(SymExprKind kind, SymExpr* e);
void     sym_expr_free(SymExpr* e);
SymExpr* sym_expr_copy(SymExpr* e);
int      sym_expr_eval(SymExpr* e, int* concrete_vals);
char*    sym_expr_to_string(SymExpr* e);
SymExpr* sym_expr_simplify(SymExpr* e);

/* ================================================================
 * Concolic Engine API
 * ================================================================ */
void concolic_init(ConcolicEngine* eng, ExploreStrategy strategy, int max_iter);
ConState* concolic_create_state(ConcolicEngine* eng, int nvars, int* initial_values);
void concolic_add_constraint(ConState* st, PCKind kind, SymExpr* lhs, int rhs, bool taken, int branch_id);
bool concolic_check_constraint(PathConstraint* pc, int* concrete);

/* Generate next inputs by negating a constraint and solving */
bool concolic_generate_next(ConcolicEngine* eng, ConState* parent,
                             int* new_inputs);

/* Run concolic exploration on a user-provided program */
typedef void (*ConcolicProgram)(ConState* st, void* user_data);
void concolic_explore(ConcolicEngine* eng,
                       ConcolicProgram program, void* user_data,
                       int nvars, int* initial_inputs);

/* Solve a path condition to find satisfying inputs */
bool concolic_solve(PathCondition* pc, int* solution, int nvars);

/* Statistics and reporting */
void concolic_report(ConcolicEngine* eng);
void concolic_free(ConcolicEngine* eng);

/* ================================================================
 * Constraint Solver (simple integer linear constraints)
 * ================================================================ */
/* Branch-and-bound style solver for path conditions */
bool solve_constraint_system(PathCondition* pc, int* inputs, int nvars);

/* Individual constraint solving */
int  solve_linear_eq(SymExpr* lhs, int rhs, int* values);
bool check_constraint_satisfied(PathConstraint* c, int* values);

#endif
