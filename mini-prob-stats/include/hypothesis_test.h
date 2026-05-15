#ifndef HYPOTHESIS_TEST_H
#define HYPOTHESIS_TEST_H

#include <stdbool.h>

typedef struct {
    double t_statistic;
    double p_value;
    double df;
    bool reject_h0;
} TTestResult;

TTestResult ttest_one_sample(double* data, int n, double mu0);
TTestResult ttest_two_sample(double* d1, int n1, double* d2, int n2);

typedef struct {
    double chi2_statistic;
    double p_value;
    int df;
    bool reject_h0;
} Chi2Result;

Chi2Result chi2_test_independence(int** table, int rows, int cols);

typedef struct {
    double z_statistic;
    double p_value;
    bool reject_h0;
} ZTestResult;

ZTestResult ztest_proportion(int k, int n, double p0);

#endif
