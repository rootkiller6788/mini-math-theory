#include "abstract.h"
#include <stdio.h>

/* ================================================================
 * abstract_interpret_demo — Abstract Interpretation Demo
 * L8: Advanced Topics — Static Analysis by Abstract Domains
 *
 * Demonstrates:
 *   1. Sign domain operations and lattice
 *   2. Interval domain operations and arithmetic
 *   3. Congruence domain
 *   4. Abstract state operations
 *   5. Abstract transfer functions
 *   6. Fixed-point iteration for loop analysis
 * ================================================================ */

int main(void) {
    printf("====== Abstract Interpretation Demo ======\n\n");

    /* ================================================================
     * Demo 1: Sign Domain Lattice
     * ================================================================ */
    printf("--- 1. Sign Domain Lattice ---\n");
    Sign signs[] = {SIGN_BOTTOM, SIGN_NEG, SIGN_ZERO, SIGN_POS,
                    SIGN_NONNEG, SIGN_NONPOS, SIGN_NONZERO, SIGN_TOP};
    int nsigns = 8;

    printf("Sign names: ");
    for (int i = 0; i < nsigns; i++)
        printf("%s ", sign_name(signs[i]));
    printf("\n");

    /* Join examples */
    printf("join(NEG, ZERO) = %s\n", sign_name(sign_join(SIGN_NEG, SIGN_ZERO)));
    printf("join(POS, ZERO) = %s\n", sign_name(sign_join(SIGN_POS, SIGN_ZERO)));
    printf("join(NEG, POS)  = %s\n", sign_name(sign_join(SIGN_NEG, SIGN_POS)));
    printf("join(TOP, NEG)  = %s\n", sign_name(sign_join(SIGN_TOP, SIGN_NEG)));

    /* Meet examples */
    printf("meet(NONNEG, NONPOS) = %s\n", sign_name(sign_meet(SIGN_NONNEG, SIGN_NONPOS)));
    printf("meet(NONPOS, NONZERO)= %s\n", sign_name(sign_meet(SIGN_NONPOS, SIGN_NONZERO)));
    printf("meet(NONNEG, NONZERO)= %s\n", sign_name(sign_meet(SIGN_NONNEG, SIGN_NONZERO)));
    printf("\n");

    /* ================================================================
     * Demo 2: Sign Arithmetic
     * ================================================================ */
    printf("--- 2. Sign Arithmetic Transfer Functions ---\n");
    printf("sign_add(POS, POS) = %s\n", sign_name(sign_add(SIGN_POS, SIGN_POS)));
    printf("sign_add(NEG, NEG) = %s\n", sign_name(sign_add(SIGN_NEG, SIGN_NEG)));
    printf("sign_add(NEG, POS) = %s\n", sign_name(sign_add(SIGN_NEG, SIGN_POS)));
    printf("sign_mul(POS, NEG) = %s\n", sign_name(sign_mul(SIGN_POS, SIGN_NEG)));
    printf("sign_mul(NEG, NEG) = %s\n\n", sign_name(sign_mul(SIGN_NEG, SIGN_NEG)));

    /* ================================================================
     * Demo 3: Sign Filter Operations (for conditionals)
     * ================================================================ */
    printf("--- 3. Sign Filters (Guards) ---\n");
    printf("filter_lt(NONNEG, 0)  = %s\n", sign_name(sign_filter_lt(SIGN_NONNEG, 0)));
    printf("filter_gt(NONPOS, 0)  = %s\n", sign_name(sign_filter_gt(SIGN_NONPOS, 0)));
    printf("filter_eq(NONNEG, 0)  = %s\n", sign_name(sign_filter_eq(SIGN_NONNEG, 0)));
    printf("filter_eq(TOP, 5)     = %s\n\n", sign_name(sign_filter_eq(SIGN_TOP, 5)));

    /* ================================================================
     * Demo 4: Interval Domain
     * ================================================================ */
    printf("--- 4. Interval Domain ---\n");
    Interval a = interval_make(1, 10);
    Interval b = interval_make(5, 15);
    Interval c = interval_make(-3, 3);

    printf("a = "); interval_print(a); printf("\n");
    printf("b = "); interval_print(b); printf("\n");
    printf("c = "); interval_print(c); printf("\n");
    printf("join(a,b) = "); interval_print(interval_join(a, b)); printf("\n");
    printf("meet(a,b) = "); interval_print(interval_meet(a, b)); printf("\n");
    printf("\n");

    /* ================================================================
     * Demo 5: Interval Arithmetic
     * ================================================================ */
    printf("--- 5. Interval Arithmetic ---\n");
    Interval iv1 = interval_make(1, 5);
    Interval iv2 = interval_make(10, 20);
    printf("[1,5] + [10,20] = "); interval_print(interval_add(iv1, iv2)); printf("\n");
    printf("[1,5] * [10,20] = "); interval_print(interval_mul(iv1, iv2)); printf("\n");
    printf("[10,20] - [1,5] = "); interval_print(interval_sub(iv2, iv1)); printf("\n");
    printf("\n");

    /* ================================================================
     * Demo 6: Interval Widening
     * ================================================================ */
    printf("--- 6. Interval Widening (for termination) ---\n");
    Interval w1 = interval_make(1, 10);
    Interval w2 = interval_make(1, 20);
    printf("widen([1,10], [1,20]) = "); interval_print(interval_widen(w1, w2)); printf("\n");
    printf("  (lo stays 1, hi grows to +inf)\n\n");

    /* ================================================================
     * Demo 7: Congruence Domain
     * ================================================================ */
    printf("--- 7. Congruence Domain ---\n");
    Congruence cg1 = congruence_make(4, 2);  /* {..., -2, 2, 6, 10, ...} */
    Congruence cg2 = congruence_make(3, 1);  /* {..., -2, 1, 4, 7, ...} */
    printf("cg1: x ≡ 2 (mod 4)\n");
    printf("cg2: x ≡ 1 (mod 3)\n");
    printf("cg1 contains 6? %s\n", congruence_contains(cg1, 6) ? "YES" : "NO");
    printf("cg1 contains 7? %s\n", congruence_contains(cg1, 7) ? "YES" : "NO");
    printf("cg2 contains 7? %s\n\n", congruence_contains(cg2, 7) ? "YES" : "NO");

    /* ================================================================
     * Demo 8: Abstract State and Transfer
     * ================================================================ */
    printf("--- 8. Abstract State & Transfer Function ---\n");
    AbsState st;
    absstate_init(&st, 3);  /* 3 variables */
    absstate_set_interval(&st, 0, interval_make(0, 100));  /* v0 in [0,100] */
    absstate_set_interval(&st, 1, interval_make(-10, 10)); /* v1 in [-10,10] */
    absstate_set_interval(&st, 2, interval_top());          /* v2 unknown */
    absstate_print(&st);

    /* Assign v0 = v0 + v1 */
    AbsExpr* add_expr = absexpr_binary(ABS_EXPR_ADD,
        absexpr_var(0), absexpr_var(1));
    AbsStmt* assign_stmt = absstmt_assign(0, add_expr);
    AbsState out = abs_transfer(assign_stmt, &st);
    printf("After v0 := v0 + v1:\n");
    absstate_print(&out);
    absstmt_free(assign_stmt);
    printf("\n");

    /* ================================================================
     * Demo 9: Loop Fixed-Point Analysis
     * Program: x := 0; while (x < 10) { x := x + 1 }
     * Abstract interpretation: x starts in [0,0], loop converges to [0,10]
     * ================================================================ */
    printf("--- 9. Loop Fixed-Point Analysis ---\n");
    AbsState loop_st;
    absstate_init(&loop_st, 1);

    /* Initial: x := 0 */
    AbsStmt* init_x = absstmt_assign(0, absexpr_const(0));
    loop_st = abs_transfer(init_x, &loop_st);
    printf("After x := 0:\n");
    absstate_print(&loop_st);

    /* while (x < 10) x := x + 1 */
    AbsExpr* guard_lt = absexpr_binary(ABS_EXPR_LT,
        absexpr_var(0), absexpr_const(10));
    AbsStmt* inc_x = absstmt_assign(0,
        absexpr_binary(ABS_EXPR_ADD, absexpr_var(0), absexpr_const(1)));
    AbsStmt* while_loop = absstmt_while(guard_lt, inc_x, 1);

    /* Run fixed-point iteration */
    AbsState final_st = abs_fixpoint(while_loop->left, while_loop->guard, 1, &loop_st);
    printf("\nAfter while(x<10) x:=x+1 (fixpoint):\n");
    absstate_print(&final_st);

    /* Execute abstract interpretation for full program */
    AbsStmt* full_prog = absstmt_seq(init_x, while_loop);
    /* Re-initialize for fresh interpretation */
    absstate_init(&loop_st, 1);
    absstate_set_interval(&loop_st, 0, interval_make(0, 0));
    printf("\nFull program abstract interpretation:\n");
    AbsTrace* trace = abs_interpret(full_prog, &loop_st);
    printf("  Initial: "); absstate_print(trace->states);
    printf("  Final:   "); absstate_print(trace->states + 1);

    abs_trace_free(trace);
    absstmt_free(full_prog); /* owns both init_x and while_loop */

    printf("\n====== All abstract interpretation tests complete ======\n");
    return 0;
}
