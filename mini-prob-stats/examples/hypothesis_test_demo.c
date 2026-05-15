#include "hypothesis_test.h"
#include <stdio.h>

int main(void) {
    printf("=== Hypothesis Testing Demo ===\n\n");

    printf("--- One-sample t-test ---\n");
    double sample[] = {98.0, 102.0, 100.0, 97.0, 103.0,
                       99.0, 101.0, 96.0, 104.0, 100.0};
    int n = 10;

    TTestResult t1 = ttest_one_sample(sample, n, 100.0);
    printf("Testing H0: mu = 100 vs H1: mu != 100\n");
    printf("  Data:");
    for (int i = 0; i < n; i++) printf(" %.0f", sample[i]);
    printf("\n  t-statistic: %.4f\n", t1.t_statistic);
    printf("  df: %.0f\n", t1.df);
    printf("  p-value: %.6f\n", t1.p_value);
    printf("  Reject H0 (alpha=0.05): %s\n\n", t1.reject_h0 ? "YES" : "NO");

    printf("--- Two-sample t-test ---\n");
    double groupA[] = {23.0, 25.0, 21.0, 27.0, 24.0, 26.0, 22.0, 28.0};
    double groupB[] = {18.0, 20.0, 19.0, 22.0, 17.0, 21.0, 16.0, 19.0};
    int nA = 8, nB = 8;

    TTestResult t2 = ttest_two_sample(groupA, nA, groupB, nB);
    printf("Testing H0: mu_A = mu_B vs H1: mu_A != mu_B\n");
    printf("  Group A:");
    for (int i = 0; i < nA; i++) printf(" %.0f", groupA[i]);
    printf("\n  Group B:");
    for (int i = 0; i < nB; i++) printf(" %.0f", groupB[i]);
    printf("\n  t-statistic: %.4f\n", t2.t_statistic);
    printf("  df: %.0f\n", t2.df);
    printf("  p-value: %.6f\n", t2.p_value);
    printf("  Reject H0 (alpha=0.05): %s\n\n", t2.reject_h0 ? "YES" : "NO");

    printf("--- Z-test for proportion ---\n");
    int heads = 58, flips = 100;
    ZTestResult zr = ztest_proportion(heads, flips, 0.5);
    printf("Testing H0: p = 0.5 (fair coin)\n");
    printf("  Observed: %d heads in %d flips (p_hat = %.2f)\n", heads, flips, (double)heads / flips);
    printf("  z-statistic: %.4f\n", zr.z_statistic);
    printf("  p-value: %.6f\n", zr.p_value);
    printf("  Reject H0 (alpha=0.05): %s\n\n", zr.reject_h0 ? "YES" : "NO");

    printf("--- Chi-squared test of independence ---\n");
    int r0[] = {50, 30, 20};
    int r1[] = {35, 45, 20};
    int* table[] = {r0, r1};

    Chi2Result cr = chi2_test_independence(table, 2, 3);
    printf("Testing independence in 2x3 contingency table\n");
    printf("  Chi2 statistic: %.4f\n", cr.chi2_statistic);
    printf("  df: %d\n", cr.df);
    printf("  p-value: %.6f\n", cr.p_value);
    printf("  Reject H0 (alpha=0.05): %s\n", cr.reject_h0 ? "YES" : "NO");

    return 0;
}
