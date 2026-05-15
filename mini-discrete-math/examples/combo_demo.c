#include "combo.h"
#include <stdio.h>
#include <stdlib.h>

static int perm_count = 0;
static void print_perm(int* arr, int n) {
    printf("%3d: ", ++perm_count);
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");
}

static int comb_count = 0;
static void print_comb(int* arr, int r) {
    printf("%3d: ", ++comb_count);
    for (int i = 0; i < r; i++) printf("%d ", arr[i]);
    printf("\n");
}

static int part_count = 0;
static void print_partition(int* buf, int n) {
    printf("%3d: ", ++part_count);
    for (int i = 0; i < n; i++) {
        printf("%d", buf[i]);
        if (i < n - 1) printf("+");
    }
    printf("\n");
}

int main(void) {
    printf("====== 06 组合数学 ======\n\n");

    // 1. 基本计数
    printf("--- 基本计数公式 ---\n");
    printf("P(10,3) = %lld   (10选3排列)\n", permutation(10, 3));
    printf("C(10,3) = %lld   (10选3组合)\n", combination(10, 3));
    printf("P(5,5)  = %lld   (5! = 120)\n\n", permutation(5, 5));

    // 2. 帕斯卡三角
    printf("--- 帕斯卡三角 (二项式系数) ---\n");
    int nrows = 8;
    long long** pascal = malloc(nrows * sizeof(long long*));
    for (int i = 0; i < nrows; i++)
        pascal[i] = malloc(nrows * sizeof(long long));
    pascal_triangle(nrows, pascal);
    for (int i = 0; i < nrows; i++) {
        printf("n=%d: ", i);
        for (int j = 0; j <= i; j++)
            printf("%lld ", pascal[i][j]);
        printf("\n");
    }
    for (int i = 0; i < nrows; i++) free(pascal[i]);
    free(pascal);
    printf("\n");

    // 3. 排列生成
    printf("--- 所有排列 (Heap's algorithm) ---\n");
    perm_count = 0;
    int perm_arr[] = {1, 2, 3, 4};
    permutations_generate(perm_arr, 4, print_perm);
    printf("共 %d 个排列\n\n", perm_count);

    // 4. 组合生成
    printf("--- 所有组合 C(5,3) ---\n");
    comb_count = 0;
    int comb_arr[] = {1, 2, 3, 4, 5};
    combinations_generate(comb_arr, 5, 3, print_comb);
    printf("共 %d 个组合\n\n", comb_count);

    // 5. 斯特林数
    printf("--- 第二类斯特林数 S(n,k) ---\n");
    printf("S(5,2) = 将 5 个元素分成 2 个非空子集的方法数 = ");
    long long s5_2;
    stirling_numbers_second(5, 2, &s5_2);
    printf("%lld\n", s5_2);
    printf("S(4,2) = ");
    long long s4_2;
    stirling_numbers_second(4, 2, &s4_2);
    printf("%lld\n\n", s4_2);

    // 6. Catalan 数
    printf("--- Catalan 数 ---\n");
    printf("C0..C10: ");
    for (int i = 0; i <= 10; i++)
        printf("%lld ", catalan_number(i));
    printf("\n(应用: 合法括号序列数, 二叉树形态数)\n\n");

    // 7. 整数拆分
    printf("--- 整数拆分 ---\n");
    printf("p(5) = %lld (5 的拆分数)\n", partitions_of_integer(5));
    printf("p(10) = %lld (10 的拆分数)\n\n", partitions_of_integer(10));

    printf("5 的所有拆分:\n");
    part_count = 0;
    int buf[128];
    partitions_enumerate(5, 5, buf, 0, print_partition);
    printf("\n");

    // 8. 错排
    printf("--- 错排 (Derangement) !n ---\n");
    printf("!1 = %lld\n", derangements(1));
    printf("!2 = %lld\n", derangements(2));
    printf("!3 = %lld\n", derangements(3));
    printf("!4 = %lld\n", derangements(4));
    printf("!5 = %lld\n", derangements(5));

    return 0;
}
