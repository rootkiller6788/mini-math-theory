#include "concolic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================
 * concolic_test_demo — Concolic Testing Demo
 * L8: Advanced Topics — Dynamic Symbolic Execution
 *
 * Demonstrates:
 *   1. Symbolic expression construction and evaluation
 *   2. Constraint checking and solving
 *   3. Concolic engine with different exploration strategies
 *   4. Automatic test case generation
 *   5. Path exploration for a sample program
 * ================================================================ */

/* Sample test program: computes absolute value */
static void abs_program(ConState* st, void* user_data) {
    (void)user_data;
    int x = st->concrete[0];

    /* if (x > 0) — branch 0 */
    concolic_add_constraint(st, PC_GT, sym_expr_var(0), 0, x > 0, 0);
    if (x > 0) {
        /* result = x — already positive */
    } else {
        /* result = -x — negate */
        st->concrete[0] = -x;
    }
    /* Postcondition: result >= 0 (sanity check) */
    if (st->concrete[0] < 0) {
        st->error_code = 1;  /* violation */
    }
    st->terminated = true;
}

/* Sample test program: sign classification with boundary checks */
static void sign_classify_program(ConState* st, void* user_data) {
    (void)user_data;
    int x = st->concrete[0];
    int y = st->concrete[1];

    /* Branch 0: x > 0 */
    concolic_add_constraint(st, PC_GT, sym_expr_var(0), 0, x > 0, 0);
    if (x > 0) {
        /* Branch 1: y > 0 */
        concolic_add_constraint(st, PC_GT, sym_expr_var(1), 0, y > 0, 1);
        if (y > 0) {
            /* Both positive: OK */
        } else if (y < 0) {
            /* pos/neg: OK */
        } else {
            st->concrete[0] = 0; /* y==0 edge case */
        }
    } else if (x < 0) {
        /* x negative: special path */
        if (x < -100) {
            st->error_code = 2; /* extreme negative */
        }
    } else {
        /* x == 0: boundary */
        st->concrete[1] = 0;
    }
    st->terminated = true;
}

int main(void) {
    printf("====== Concolic Testing Demo ======\n\n");

    /* ================================================================
     * Demo 1: Symbolic Expressions
     * ================================================================ */
    printf("--- 1. Symbolic Expressions ---\n");
    SymExpr* e1 = sym_expr_const(42);
    printf("const(42) = %s\n", sym_expr_to_string(e1));

    SymExpr* e2 = sym_expr_var(0);
    printf("sym(0) = %s\n", sym_expr_to_string(e2));

    SymExpr* e3 = sym_expr_binary(SEXPR_ADD,
        sym_expr_var(0), sym_expr_const(3));
    printf("x0 + 3 = %s\n", sym_expr_to_string(e3));

    SymExpr* e4 = sym_expr_binary(SEXPR_MUL,
        sym_expr_var(0), sym_expr_var(1));
    printf("x0 * x1 = %s\n", sym_expr_to_string(e4));

    /* Evaluate with concrete values */
    int vals[] = {5, 10, 0};
    printf("eval(x0+3, {5,10}) = %d\n", sym_expr_eval(e3, vals));
    printf("eval(x0*x1, {5,10}) = %d\n", sym_expr_eval(e4, vals));
    sym_expr_free(e1); sym_expr_free(e2);
    sym_expr_free(e3); sym_expr_free(e4);
    printf("\n");

    /* ================================================================
     * Demo 2: Constraint Checking
     * ================================================================ */
    printf("--- 2. Constraint Checking ---\n");
    PathConstraint pc1;
    pc1.kind = PC_GT;
    pc1.lhs = sym_expr_var(0);
    pc1.rhs = sym_expr_const(0);
    pc1.taken = true;
    pc1.branch_id = 0;

    int v1[] = {5, 0};
    int v2[] = {-3, 0};
    printf("x0 > 0 with x0=5:  %s\n",
           check_constraint_satisfied(&pc1, v1) ? "SAT" : "UNSAT");
    printf("x0 > 0 with x0=-3: %s\n",
           check_constraint_satisfied(&pc1, v2) ? "SAT" : "UNSAT");
    sym_expr_free(pc1.lhs);
    sym_expr_free(pc1.rhs);
    printf("\n");

    /* ================================================================
     * Demo 3: Constraint Solving
     * ================================================================ */
    printf("--- 3. Constraint System Solving ---\n");
    PathCondition pc_sys;
    pc_sys.count = 2;
    pc_sys.constraints[0].kind = PC_GT;
    pc_sys.constraints[0].lhs = sym_expr_var(0);
    pc_sys.constraints[0].rhs = sym_expr_const(0);
    pc_sys.constraints[0].taken = true;
    pc_sys.constraints[0].branch_id = 0;

    pc_sys.constraints[1].kind = PC_LT;
    pc_sys.constraints[1].lhs = sym_expr_var(0);
    pc_sys.constraints[1].rhs = sym_expr_const(10);
    pc_sys.constraints[1].taken = true;
    pc_sys.constraints[1].branch_id = 1;

    int sol[] = {0, 0};
    if (solve_constraint_system(&pc_sys, sol, 2)) {
        printf("Solved: x0 = %d (0 < x0 < 10)\n", sol[0]);
    } else {
        printf("Unsatisfiable\n");
    }
    sym_expr_free(pc_sys.constraints[0].lhs);
    sym_expr_free(pc_sys.constraints[0].rhs);
    sym_expr_free(pc_sys.constraints[1].lhs);
    sym_expr_free(pc_sys.constraints[1].rhs);
    printf("\n");

    /* ================================================================
     * Demo 4: Concolic Engine — Absolute Value
     * ================================================================ */
    printf("--- 4. Concolic Exploration: abs(x) ---\n");
    ConcolicEngine eng;
    concolic_init(&eng, EXPLORE_DFS, 10);
    int init_abs[] = {5};  /* start with x=5 */
    concolic_explore(&eng, abs_program, NULL, 1, init_abs);
    concolic_free(&eng);
    printf("\n");

    /* ================================================================
     * Demo 5: Concolic Engine — Sign Classification (BFS)
     * ================================================================ */
    printf("--- 5. Concolic Exploration: sign_classify(x, y) [BFS] ---\n");
    concolic_init(&eng, EXPLORE_BFS, 15);
    int init_sign[] = {10, 20};  /* start with x=10, y=20 */
    concolic_explore(&eng, sign_classify_program, NULL, 2, init_sign);
    concolic_free(&eng);
    printf("\n");

    /* ================================================================
     * Demo 6: Concolic Engine — Random Strategy
     * ================================================================ */
    printf("--- 6. Concolic Exploration with Random Strategy ---\n");
    concolic_init(&eng, EXPLORE_RANDOM, 8);
    int init_rnd[] = {7, -3};
    concolic_explore(&eng, sign_classify_program, NULL, 2, init_rnd);
    concolic_free(&eng);
    printf("\n");

    /* ================================================================
     * Demo 7: Expression Simplification
     * ================================================================ */
    printf("--- 7. Symbolic Expression Simplification ---\n");
    SymExpr* se1 = sym_expr_binary(SEXPR_ADD,
        sym_expr_const(3), sym_expr_const(4));
    SymExpr* se1s = sym_expr_simplify(se1);
    printf("3+4 => %s\n", sym_expr_to_string(se1s));
    sym_expr_free(se1); sym_expr_free(se1s);

    SymExpr* se2 = sym_expr_binary(SEXPR_MUL,
        sym_expr_binary(SEXPR_ADD, sym_expr_const(2), sym_expr_const(3)),
        sym_expr_const(4));
    SymExpr* se2s = sym_expr_simplify(se2);
    printf("(2+3)*4 => %s\n\n", sym_expr_to_string(se2s));
    sym_expr_free(se2); sym_expr_free(se2s);

    printf("====== All concolic testing tests complete ======\n");
    return 0;
}
