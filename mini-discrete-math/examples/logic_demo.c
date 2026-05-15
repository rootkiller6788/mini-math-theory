#include "logic.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("====== 01 逻辑与证明 ======\n\n");

    // 1. 命题逻辑真值表: (p ∧ q) → r
    printf("--- 真值表: (A ∧ B) → C ---\n");
    Formula* f1 = formula_create();
    f1->nvars = 3;
    f1->var_names[0] = 'A';
    f1->var_names[1] = 'B';
    f1->var_names[2] = 'C';
    int a = formula_add_var(f1, 0);
    int b = formula_add_var(f1, 1);
    int c = formula_add_var(f1, 2);
    int a_and_b = formula_add_and(f1, a, b);
    int root = formula_add_implies(f1, a_and_b, c);
    truth_table_print(f1, root);
    printf("\n");

    // 2. 检查重言式: p ∨ ¬p
    printf("--- 重言式检查: A ∨ ¬A ---\n");
    Formula* f2 = formula_create();
    f2->nvars = 1;
    f2->var_names[0] = 'A';
    int p = formula_add_var(f2, 0);
    int not_p = formula_add_not(f2, p);
    int root2 = formula_add_or(f2, p, not_p);
    truth_table_print(f2, root2);
    printf("是重言式? %s\n\n", is_tautology(f2, root2) ? "是" : "否");

    // 3. 检查矛盾式: p ∧ ¬p
    printf("--- 矛盾式检查: A ∧ ¬A ---\n");
    Formula* f3 = formula_create();
    f3->nvars = 1;
    f3->var_names[0] = 'A';
    int q = formula_add_var(f3, 0);
    int not_q = formula_add_not(f3, q);
    int root3 = formula_add_and(f3, q, not_q);
    printf("是矛盾式? %s\n\n", is_contradiction(f3, root3) ? "是" : "否");

    // 4. 数学归纳法
    proof_by_induction_demo(10);

    free(f1);
    free(f2);
    free(f3);
    return 0;
}
