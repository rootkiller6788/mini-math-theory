#include "hypothesis_test.h"
#include "inference.h"
#include "distribution.h"
#include <math.h>
#include <stdlib.h>

static double normal_cdf_approx(double x) {
    return 0.5 * (1.0 + erf(x / sqrt(2.0)));
}

static double compute_p_value_normal(double statistic) {
    double absz = fabs(statistic);
    return 2.0 * (1.0 - normal_cdf_approx(absz));
}

TTestResult ttest_one_sample(double* data, int n, double mu0) {
    TTestResult res = {0.0, 1.0, 0.0, false};
    if (n <= 1) return res;

    double xbar = mean(data, n);
    double ss = 0.0;
    for (int i = 0; i < n; i++) {
        double d = data[i] - xbar;
        ss += d * d;
    }
    double s = sqrt(ss / (n - 1));
    double se = s / sqrt((double)n);

    res.t_statistic = (xbar - mu0) / se;
    res.df = n - 1;
    res.p_value = compute_p_value_normal(res.t_statistic);
    res.reject_h0 = (res.p_value < 0.05);
    return res;
}

TTestResult ttest_two_sample(double* d1, int n1, double* d2, int n2) {
    TTestResult res = {0.0, 1.0, 0.0, false};
    if (n1 <= 1 || n2 <= 1) return res;

    double xbar1 = mean(d1, n1);
    double xbar2 = mean(d2, n2);
    double ss1 = 0.0, ss2 = 0.0;
    for (int i = 0; i < n1; i++) { double d = d1[i] - xbar1; ss1 += d * d; }
    for (int i = 0; i < n2; i++) { double d = d2[i] - xbar2; ss2 += d * d; }

    double sp2 = (ss1 + ss2) / (n1 + n2 - 2);
    double se = sqrt(sp2 * (1.0 / n1 + 1.0 / n2));

    res.t_statistic = (xbar1 - xbar2) / se;
    res.df = n1 + n2 - 2;
    res.p_value = compute_p_value_normal(res.t_statistic);
    res.reject_h0 = (res.p_value < 0.05);
    return res;
}

static double wilson_hilferty_pvalue(double chi2, int df) {
    if (df <= 0) return 1.0;
    if (chi2 <= 0.0) return 1.0;
    double r = chi2 / df;
    double c1 = 2.0 / (9.0 * df);
    double z = (pow(r, 1.0 / 3.0) - (1.0 - c1)) / sqrt(c1);
    return 1.0 - normal_cdf_approx(z);
}

Chi2Result chi2_test_independence(int** table, int rows, int cols) {
    Chi2Result res = {0.0, 1.0, 0, false};
    if (rows <= 0 || cols <= 0 || !table) return res;

    int* row_sum = (int*)calloc(rows, sizeof(int));
    int* col_sum = (int*)calloc(cols, sizeof(int));
    int grand = 0;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            row_sum[i] += table[i][j];
            col_sum[j] += table[i][j];
            grand += table[i][j];
        }
    }

    if (grand == 0) {
        free(row_sum); free(col_sum);
        return res;
    }

    double chi2 = 0.0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double expected = (double)row_sum[i] * col_sum[j] / grand;
            if (expected > 0.0) {
                double diff = table[i][j] - expected;
                chi2 += (diff * diff) / expected;
            }
        }
    }

    res.df = (rows - 1) * (cols - 1);
    res.chi2_statistic = chi2;
    if (res.df > 0) {
        res.p_value = wilson_hilferty_pvalue(chi2, res.df);
    } else {
        res.p_value = 1.0;
    }
    res.reject_h0 = (res.p_value < 0.05);

    free(row_sum);
    free(col_sum);
    return res;
}

ZTestResult ztest_proportion(int k, int n, double p0) {
    ZTestResult res = {0.0, 1.0, false};
    if (n <= 0) return res;

    double p_hat = (double)k / n;
    double se = sqrt(p0 * (1.0 - p0) / n);
    if (se == 0.0) {
        if (fabs(p_hat - p0) < 1e-10) {
            res.z_statistic = 0.0;
            res.p_value = 1.0;
        } else {
            res.z_statistic = (p_hat > p0) ? 1e10 : -1e10;
            res.p_value = 0.0;
        }
        res.reject_h0 = (res.p_value < 0.05);
        return res;
    }

    res.z_statistic = (p_hat - p0) / se;
    res.p_value = 2.0 * (1.0 - normal_cdf_approx(fabs(res.z_statistic)));
    res.reject_h0 = (res.p_value < 0.05);
    return res;
}
