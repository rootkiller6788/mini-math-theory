#include "set.h"
#include "relation.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void print_perm(int* arr, int n) {
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");
}

int main(void) {
    printf("====== 03 集合与关系 ======\n\n");

    // === 集 合 ===
    printf("--- 集合基本运算 ---\n");
    Set A, B;
    set_init(&A); set_init(&B);

    int arrA[] = {1, 3, 5, 7, 9};
    int arrB[] = {3, 6, 7, 9, 12};
    for (int i = 0; i < 5; i++) { set_add(&A, arrA[i]); set_add(&B, arrB[i]); }

    printf("A = "); set_print(&A); printf("\n");
    printf("B = "); set_print(&B); printf("\n");

    Set U = set_union(&A, &B);
    printf("A ∪ B = "); set_print(&U); printf("\n");

    Set I = set_intersection(&A, &B);
    printf("A ∩ B = "); set_print(&I); printf("\n");

    Set D = set_difference(&A, &B);
    printf("A - B = "); set_print(&D); printf("\n");

    printf("A ⊆ B? %s\n\n", set_subset(&A, &B) ? "是" : "否");

    // 幂集
    printf("--- 幂集 P({1,2,3}) ---\n");
    Set S; set_init(&S);
    set_add(&S, 1); set_add(&S, 2); set_add(&S, 3);
    PowerSet ps = power_set_generate(&S);
    printf("Base: |P(S)| = %d\n", ps.count);
    for (int i = 0; i < ps.count; i++) {
        set_print(&ps.sets[i]); printf(" ");
    }
    printf("\n\n");

    // 笛卡尔积
    printf("--- 笛卡尔积 A × B ---\n");
    Set X, Y;
    set_init(&X); set_init(&Y);
    set_add(&X, 1); set_add(&X, 2);
    set_add(&Y, 3); set_add(&Y, 4);
    CartesianProduct cp = cartesian_product(&X, &Y);
    printf("{");
    for (int i = 0; i < cp.size; i++) {
        printf("(%d,%d)", cp.pairs[i].first, cp.pairs[i].second);
        if (i < cp.size - 1) printf(", ");
    }
    printf("}\n\n");

    // === 关 系 ===
    printf("--- 关系矩阵 ---\n");
    Relation R;
    relation_init(&R, 5);
    relation_add_pair(&R, 0, 0);
    relation_add_pair(&R, 1, 1);
    relation_add_pair(&R, 2, 2);
    relation_add_pair(&R, 3, 3);
    relation_add_pair(&R, 4, 4);
    relation_add_pair(&R, 0, 1);
    relation_add_pair(&R, 1, 0);
    relation_add_pair(&R, 1, 2);
    relation_add_pair(&R, 2, 1);
    relation_add_pair(&R, 3, 4);

    printf("关系 R (论域 {0..4}):\n");
    relation_print_matrix(&R);
    relation_print_pairs(&R);

    printf("自反? %s\n", relation_is_reflexive(&R) ? "是" : "否");
    printf("对称? %s\n", relation_is_symmetric(&R) ? "是" : "否");
    printf("传递? %s\n", relation_is_transitive(&R) ? "是" : "否");
    printf("等价? %s\n\n", relation_is_equivalence(&R) ? "是" : "否");

    // 等价关系: 划分
    printf("--- 等价关系与划分 ---\n");
    Relation E;
    relation_init(&E, 4);
    // 模 2 同余关系
    relation_add_pair(&E, 0, 0); relation_add_pair(&E, 0, 2);
    relation_add_pair(&E, 1, 1); relation_add_pair(&E, 1, 3);
    relation_add_pair(&E, 2, 0); relation_add_pair(&E, 2, 2);
    relation_add_pair(&E, 3, 1); relation_add_pair(&E, 3, 3);

    printf("等价关系 E (x≡y mod 2):\n");
    relation_print_matrix(&E);
    printf("是等价? %s\n", relation_is_equivalence(&E) ? "是" : "否");

    EquivalencePartition ep = equivalence_classes(&E);
    printf("等价类划分 (%d 个类):\n", ep.nclasses);
    for (int i = 0; i < ep.nclasses; i++) {
        printf("  [");
        for (int j = 0; j < ep.class_sizes[i]; j++)
            printf("%d%s", ep.classes[i][j], j < ep.class_sizes[i] - 1 ? ", " : "");
        printf("]\n");
    }
    printf("\n");

    // 偏序 + 拓扑排序
    printf("--- 偏序关系与拓扑排序 ---\n");
    Relation PO;
    relation_init(&PO, 6);
    // 整除关系 poset
    relation_add_pair(&PO, 0, 0); // 1 reflect
    relation_add_pair(&PO, 1, 1);
    relation_add_pair(&PO, 2, 2);
    relation_add_pair(&PO, 3, 3);
    relation_add_pair(&PO, 4, 4);
    relation_add_pair(&PO, 5, 5);
    // 1 divides everything
    relation_add_pair(&PO, 0, 1); relation_add_pair(&PO, 0, 2);
    relation_add_pair(&PO, 0, 3); relation_add_pair(&PO, 0, 4);
    relation_add_pair(&PO, 0, 5);
    // 2 divides 4
    relation_add_pair(&PO, 1, 3); relation_add_pair(&PO, 1, 5);
    // 3 divides 5
    relation_add_pair(&PO, 2, 5);

    printf("偏序 (1|2, 1|3, 1|4, 1|6, 2|4, 2|6, 3|6):\n");
    relation_print_matrix(&PO);
    printf("是偏序? %s\n", relation_is_partial_order(&PO) ? "是" : "否");

    int* topo = topological_sort(&PO);
    printf("拓扑序: ");
    for (int i = 0; i < PO.size; i++) printf("%d ", topo[i]);
    printf("\n");

    free(topo);
    return 0;
}
