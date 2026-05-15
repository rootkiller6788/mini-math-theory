#include <stdio.h>
#include "../include/sat.h"

static void print_assignment(const int *assign, int n) {
    printf("  { ");
    for (int i = 1; i <= n; i++) {
        if (assign[i] == 1) printf("x%d=T ", i);
        else if (assign[i] == -1) printf("x%d=F ", i);
    }
    printf("}\n");
}

int main(void) {
    printf("=== SAT Solver Demo (DPLL) ===\n\n");

    /* Test 1: Simple satisfiable 2-CNF formula
     * (x1 ∨ x2) ∧ (¬x1 ∨ x3) ∧ (¬x2 ∨ ¬x3)
     * Satisfying: x1=F, x2=T, x3=T  or  x1=T, x2=F, x3=T etc. */
    {
        printf("--- Test 1: SAT (2-CNF) ---\n");
        SAT sat = sat_create(3);
        int c0 = sat_add_clause(&sat);
        sat_add_literal(&sat, c0, 1);
        sat_add_literal(&sat, c0, 2);
        int c1 = sat_add_clause(&sat);
        sat_add_literal(&sat, c1, -1);
        sat_add_literal(&sat, c1, 3);
        int c2 = sat_add_clause(&sat);
        sat_add_literal(&sat, c2, -2);
        sat_add_literal(&sat, c2, -3);
        sat_print(&sat);

        int assign[MAX_SAT_VARS + 1];
        if (sat_solve(&sat, assign)) {
            printf("SATISFIABLE\n");
            print_assignment(assign, sat.num_vars);
        } else {
            printf("UNSATISFIABLE\n");
        }
        printf("\n");
    }

    /* Test 2: Unsatisfiable (pigeonhole-like)
     * (x1) ∧ (¬x1 ∨ x2) ∧ (¬x2 ∨ x3) ∧ (¬x3) */
    {
        printf("--- Test 2: UNSAT ---\n");
        SAT sat = sat_create(3);
        int c0 = sat_add_clause(&sat);
        sat_add_literal(&sat, c0, 1);              /* x1 */
        int c1 = sat_add_clause(&sat);
        sat_add_literal(&sat, c1, -1);
        sat_add_literal(&sat, c1, 2);              /* ¬x1 ∨ x2 */
        int c2 = sat_add_clause(&sat);
        sat_add_literal(&sat, c2, -2);
        sat_add_literal(&sat, c2, 3);              /* ¬x2 ∨ x3 */
        int c3 = sat_add_clause(&sat);
        sat_add_literal(&sat, c3, -3);             /* ¬x3 */
        sat_print(&sat);

        int assign[MAX_SAT_VARS + 1];
        if (sat_solve(&sat, assign)) {
            printf("SATISFIABLE\n");
            print_assignment(assign, sat.num_vars);
        } else {
            printf("UNSATISFIABLE\n");
        }
        printf("\n");
    }

    /* Test 3: 3-SAT example
     * (x1 ∨ x2 ∨ x3) ∧ (¬x1 ∨ ¬x2) ∧ (¬x1 ∨ ¬x3) ∧ (¬x2 ∨ ¬x3)
     * Satisfying: one variable true, others false. */
    {
        printf("--- Test 3: 3-SAT ---\n");
        SAT sat = sat_create(3);
        int c0 = sat_add_clause(&sat);
        sat_add_literal(&sat, c0, 1);
        sat_add_literal(&sat, c0, 2);
        sat_add_literal(&sat, c0, 3);
        int c1 = sat_add_clause(&sat);
        sat_add_literal(&sat, c1, -1);
        sat_add_literal(&sat, c1, -2);
        int c2 = sat_add_clause(&sat);
        sat_add_literal(&sat, c2, -1);
        sat_add_literal(&sat, c2, -3);
        int c3 = sat_add_clause(&sat);
        sat_add_literal(&sat, c3, -2);
        sat_add_literal(&sat, c3, -3);
        sat_print(&sat);

        int assign[MAX_SAT_VARS + 1];
        if (sat_solve(&sat, assign)) {
            printf("SATISFIABLE\n");
            print_assignment(assign, sat.num_vars);
        } else {
            printf("UNSATISFIABLE\n");
        }
        printf("\n");
    }

    /* Test 4: Pure literal test
     * (x1 ∨ x2) ∧ (x1 ∨ ¬x3)  → x1 is pure positive */
    {
        printf("--- Test 4: Pure Literal ---\n");
        SAT sat = sat_create(3);
        int c0 = sat_add_clause(&sat);
        sat_add_literal(&sat, c0, 1);
        sat_add_literal(&sat, c0, 2);
        int c1 = sat_add_clause(&sat);
        sat_add_literal(&sat, c1, 1);
        sat_add_literal(&sat, c1, -3);
        sat_print(&sat);

        int assign[MAX_SAT_VARS + 1];
        if (sat_solve(&sat, assign)) {
            printf("SATISFIABLE\n");
            print_assignment(assign, sat.num_vars);
        } else {
            printf("UNSATISFIABLE\n");
        }
        printf("\n");
    }

    return 0;
}
