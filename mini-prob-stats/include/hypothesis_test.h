#ifndef HYPOTHESIS_TEST_H
#define HYPOTHESIS_TEST_H

#include <stdbool.h>

/* L1: Struct for t-test results */
typedef struct {
    double t_statistic;
    double p_value;
    double df;
    bool reject_h0;
} TTestResult;

/* L5: t-tests */
TTestResult ttest_one_sample(double* data, int n, double mu0);
TTestResult ttest_two_sample(double* d1, int n1, double* d2, int n2);
TTestResult ttest_paired(double* x, double* y, int n);
TTestResult ttest_welch(double* d1, int n1, double* d2, int n2);

/* L1: Struct for chi-squared test results */
typedef struct {
    double chi2_statistic;
    double p_value;
    int df;
    bool reject_h0;
} Chi2Result;

/* L5: Chi-squared tests */
Chi2Result chi2_test_independence(int** table, int rows, int cols);
Chi2Result chi2_goodness_of_fit(int* observed, double* expected, int n_bins,
                                 int n_params_estimated);

/* L1: Struct for Z-test results */
typedef struct {
    double z_statistic;
    double p_value;
    bool reject_h0;
} ZTestResult;

/* L5: Z-tests */
ZTestResult ztest_proportion(int k, int n, double p0);
ZTestResult ztest_two_proportions(int k1, int n1, int k2, int n2);

/* L1: Struct for ANOVA results */
typedef struct {
    double f_statistic;
    double p_value;
    int df_between;
    int df_within;
    bool reject_h0;
} AnovaResult;

/* L5: One-way ANOVA */
AnovaResult anova_oneway(double** groups, int* sizes, int k);

/* L7: Kolmogorov-Smirnov two-sample test
 * Returns D statistic, stores p-value in *p_value */
double ks_test_two_sample(double* d1, int n1, double* d2, int n2, double* p_value);

/* L7: Mann-Whitney U test
 * Returns U statistic, stores p-value in *p_value */
double mann_whitney_u(double* d1, int n1, double* d2, int n2, double* p_value);

/* L4: Power analysis */
double test_power_gaussian(int n, double mu0, double mu1, double sigma, double alpha);
double sample_size_for_power(double mu0, double mu1, double sigma,
                              double alpha, double desired_power);

#endif
