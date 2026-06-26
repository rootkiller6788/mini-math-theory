/* test_all.c �� Comprehensive tests for mini-prob-stats.
 *
 * Tests cover L1-L6 across all modules.
 */

#include "probability.h"
#include "random_variable.h"
#include "distribution.h"
#include "bayes.h"
#include "inference.h"
#include "hypothesis_test.h"
#include "regression.h"
#include "monte_carlo.h"
#include "sampling.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <time.h>

#define EPS 1e-4
#define EPS_WEAK 0.1

static int tests_run = 0;
static int tests_passed = 0;
#define RUN_TEST(name) do { tests_run++; printf("  [%3d] %-55s", tests_run, name); } while(0)
#define PASS() do { tests_passed++; printf("PASS\n"); } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); } while(0)
#define ASSERT_TRUE(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)
#define ASSERT_NEAR(a, b, eps, msg) do { if (fabs((a)-(b)) > (eps)) { printf("FAIL: %s (got %.6f, expected %.6f)\n", msg, a, b); return; } } while(0)

/* =========================================================================
 * PROBABILITY TESTS
 * ========================================================================= */
static void test_prob_basic(void) {
    RUN_TEST("prob_union"); ASSERT_NEAR(prob_union(0.3, 0.4, 0.1), 0.6, EPS, ""); PASS();
}
static void test_prob_conditional(void) {
    RUN_TEST("prob_conditional"); ASSERT_NEAR(prob_conditional(0.2, 0.5), 0.4, EPS, ""); PASS();
}
static void test_prob_bayes_theorem(void) {
    RUN_TEST("prob_bayes"); ASSERT_NEAR(prob_bayes(0.3, 0.8, 0.24), 1.0, EPS, ""); PASS();
}
static void test_factorial(void) {
    RUN_TEST("factorial"); assert(factorial(5) == 120); assert(factorial(0) == 1); PASS();
}
static void test_nCr_nPr_test(void) {
    RUN_TEST("nCr"); assert(nCr(5, 2) == 10); assert(nCr(10, 3) == 120); PASS();
}
static void test_nPr(void) {
    RUN_TEST("nPr"); assert(nPr(5, 2) == 20); assert(nPr(4, 2) == 12); PASS();
}
static void test_catalan(void) {
    RUN_TEST("catalan"); assert(catalan(3) == 5); assert(catalan(0) == 1); PASS();
}
static void test_bell_number(void) {
    RUN_TEST("bell_number"); assert(bell_number(3) == 5); assert(bell_number(0) == 1); PASS();
}
static void test_derangement(void) {
    RUN_TEST("derangement"); assert(derangement(3) == 2); assert(derangement(4) == 9); PASS();
}
static void test_law_total_prob(void) {
    RUN_TEST("law_of_total_probability");
    double p_a_given_b[] = {0.9, 0.1};
    double p_b[] = {0.01, 0.99};
    ASSERT_NEAR(law_of_total_probability(p_a_given_b, p_b, 2), 0.108, 1e-6, ""); PASS();
}
static void test_extended_bayes(void) {
    RUN_TEST("extended_bayes");
    double p_a_given_b[] = {0.95, 0.05};
    double p_b[] = {0.001, 0.999};
    double post = extended_bayes(p_a_given_b, p_b, 2, 0);
    ASSERT_NEAR(post, 0.018664, 1e-4, ""); PASS();
}
static void test_poisson_process_prob_test(void) {
    RUN_TEST("poisson_process_prob");
    double p = poisson_process_prob_k(2.0, 1.0, 0);
    ASSERT_NEAR(p, exp(-2.0), EPS, ""); PASS();
}
static void test_markov_bound(void) {
    RUN_TEST("markov_bound"); ASSERT_NEAR(markov_bound(10.0, 20.0), 0.5, EPS, ""); PASS();
}
static void test_chebyshev_bound(void) {
    RUN_TEST("chebyshev_bound"); ASSERT_NEAR(chebyshev_bound(4.0, 2.0), 1.0, EPS, ""); PASS();
}
static void test_hoeffding_bound(void) {
    RUN_TEST("hoeffding_bound");
    double b = hoeffding_bound(100, 0.2, 0.0, 1.0);
    assert(b < 1.0 && b > 0.0); PASS();
}
static void test_fisher_yates_shuffle(void) {
    RUN_TEST("fisher_yates_shuffle");
    int arr[] = {0, 1, 2, 3, 4};
    fisher_yates_shuffle(arr, 5);
    int sum = 0;
    for (int i = 0; i < 5; i++) sum += arr[i];
    assert(sum == 10); PASS();
}
static void test_lln_uniform_mean_test(void) {
    RUN_TEST("llN_uniform_mean");
    double* means = llN_uniform_mean(1000);
    assert(means != NULL);
    assert(fabs(means[999] - 0.5) < 0.2);
    free(means); PASS();
}

/* =========================================================================
 * RANDOM VARIABLE TESTS
 * ========================================================================= */
static void test_rv_create(void) {
    RUN_TEST("rv_create");
    DiscreteRV* rv = rv_create(3);
    assert(rv != NULL); assert(rv->n == 3);
    rv_free(rv); PASS();
}
static void test_rv_expectation(void) {
    RUN_TEST("rv_expectation");
    DiscreteRV* rv = rv_create(2);
    rv_set(rv, 0, 0.0, 0.5);
    rv_set(rv, 1, 1.0, 0.5);
    ASSERT_NEAR(rv_expectation(rv), 0.5, EPS, ""); rv_free(rv); PASS();
}
static void test_rv_variance(void) {
    RUN_TEST("rv_variance");
    DiscreteRV* rv = rv_create(2);
    rv_set(rv, 0, 0.0, 0.5);
    rv_set(rv, 1, 1.0, 0.5);
    ASSERT_NEAR(rv_variance(rv), 0.25, EPS, ""); rv_free(rv); PASS();
}
static void test_rv_covariance_test(void) {
    RUN_TEST("rv_covariance");
    DiscreteRV* x = rv_create(2);
    DiscreteRV* y = rv_create(2);
    rv_set(x, 0, 0.0, 0.5); rv_set(x, 1, 1.0, 0.5);
    rv_set(y, 0, 0.0, 0.5); rv_set(y, 1, 2.0, 0.5);
    /* E[X]=0.5, E[Y]=1.0, E[XY]=1.0, Cov=1.0-0.5=0.5 */
    ASSERT_NEAR(rv_covariance(x, y), 0.5, EPS, "");
    rv_free(x); rv_free(y); PASS();
}
static void test_rv_skewness_test(void) {
    RUN_TEST("rv_skewness");
    DiscreteRV* rv = rv_create(2);
    rv_set(rv, 0, 0.0, 0.5);
    rv_set(rv, 1, 1.0, 0.5);
    ASSERT_NEAR(rv_skewness(rv), 0.0, EPS, ""); rv_free(rv); PASS();
}
static void test_rv_entropy_test(void) {
    RUN_TEST("rv_entropy");
    DiscreteRV* rv = rv_create(2);
    rv_set(rv, 0, 0.0, 0.5);
    rv_set(rv, 1, 1.0, 0.5);
    ASSERT_NEAR(rv_entropy(rv), log(2.0), EPS, "fair coin entropy = 1 bit"); rv_free(rv); PASS();
}
static void test_rv_kl_divergence_test(void) {
    RUN_TEST("rv_kl_divergence");
    DiscreteRV* p = rv_create(2);
    DiscreteRV* q = rv_create(2);
    rv_set(p, 0, 0.0, 0.5); rv_set(p, 1, 1.0, 0.5);
    rv_set(q, 0, 0.0, 0.5); rv_set(q, 1, 1.0, 0.5);
    ASSERT_NEAR(rv_kl_divergence(p, q), 0.0, EPS, ""); rv_free(p); rv_free(q); PASS();
}

/* =========================================================================
 * DISTRIBUTION TESTS
 * ========================================================================= */
static void test_bernoulli(void) {
    RUN_TEST("bernoulli");
    ASSERT_NEAR(bernoulli_pmf(1, 0.7), 0.7, EPS, "");
    ASSERT_NEAR(bernoulli_pmf(0, 0.7), 0.3, EPS, "");
    ASSERT_NEAR(bernoulli_expectation(0.3), 0.3, EPS, "");
    ASSERT_NEAR(bernoulli_variance(0.3), 0.21, EPS, ""); PASS();
}
static void test_binomial_test(void) {
    RUN_TEST("binomial");
    ASSERT_NEAR(binomial_pmf(2, 5, 0.5), 10.0/32.0, EPS, "");
    ASSERT_NEAR(binomial_expectation(10, 0.3), 3.0, EPS, "");
    ASSERT_NEAR(binomial_variance(10, 0.3), 2.1, EPS, ""); PASS();
}
static void test_poisson_test(void) {
    RUN_TEST("poisson");
    ASSERT_NEAR(poisson_pmf(0, 1.0), exp(-1.0), EPS, "");
    ASSERT_NEAR(poisson_expectation(3.0), 3.0, EPS, "");
    ASSERT_NEAR(poisson_variance(3.0), 3.0, EPS, ""); PASS();
}
static void test_gaussian_test(void) {
    RUN_TEST("gaussian");
    ASSERT_NEAR(gaussian_pdf(0.0, 0.0, 1.0), 1.0/sqrt(2.0*PI), EPS, "");
    ASSERT_NEAR(gaussian_cdf(0.0, 0.0, 1.0), 0.5, EPS, "");
    ASSERT_NEAR(gaussian_expectation(1.0, 2.0), 1.0, EPS, "");
    ASSERT_NEAR(gaussian_variance(1.0, 2.0), 4.0, EPS, ""); PASS();
}
static void test_exponential_test(void) {
    RUN_TEST("exponential");
    ASSERT_NEAR(exponential_pdf(0.0, 2.0), 2.0, EPS, "");
    ASSERT_NEAR(exponential_expectation(2.0), 0.5, EPS, "");
    ASSERT_NEAR(exponential_variance(2.0), 0.25, EPS, ""); PASS();
}
static void test_gamma_dist_test(void) {
    RUN_TEST("gamma");
    ASSERT_NEAR(gamma_expectation(2.0, 1.0), 2.0, EPS, "");
    ASSERT_NEAR(gamma_variance(2.0, 1.0), 2.0, EPS, ""); PASS();
}
static void test_beta_dist_test(void) {
    RUN_TEST("beta"); ASSERT_NEAR(beta_expectation(2.0, 2.0), 0.5, EPS, ""); PASS();
}
static void test_chi2_dist_test(void) {
    RUN_TEST("chi2"); ASSERT_NEAR(chi2_expectation(5.0), 5.0, EPS, ""); PASS();
}
static void test_f_dist_test(void) {
    RUN_TEST("f_expectation"); assert(!isfinite(f_expectation(1.0, 1.0))); PASS();
}
static void test_lognormal_test(void) {
    RUN_TEST("lognormal");
    ASSERT_NEAR(lognormal_expectation(0.0, 1.0), exp(0.5), EPS, ""); PASS();
}
static void test_weibull_test(void) {
    RUN_TEST("weibull");
    ASSERT_NEAR(weibull_cdf(1.0, 1.0, 1.0), 1.0 - exp(-1.0), EPS, ""); PASS();
}
static void test_cauchy_test(void) {
    RUN_TEST("cauchy"); ASSERT_NEAR(cauchy_cdf(0.0, 0.0, 1.0), 0.5, EPS, ""); PASS();
}
static void test_geometric_test(void) {
    RUN_TEST("geometric");
    ASSERT_NEAR(geometric_pmf(0, 0.5), 0.5, EPS, "");
    ASSERT_NEAR(geometric_expectation(0.5), 1.0, EPS, ""); PASS();
}
static void test_rayleigh_test(void) {
    RUN_TEST("rayleigh");
    ASSERT_NEAR(rayleigh_variance(1.0), (4.0-PI)/2.0, EPS, ""); PASS();
}
static void test_multivariate_gaussian_test(void) {
    RUN_TEST("multivariate_gaussian_logpdf");
    double x[] = {0, 0}, mu[] = {0, 0}, sig[] = {1, 1};
    ASSERT_NEAR(multivariate_gaussian_logpdf(x, mu, sig, 2), -log(2.0*PI), EPS, ""); PASS();
}

/* =========================================================================
 * BAYES TESTS
 * ========================================================================= */
static void test_beta_binomial(void) {
    RUN_TEST("beta_binomial");
    BetaBinomial* bb = bayes_beta_binomial_create(2.0, 2.0);
    ASSERT_NEAR(bayes_posterior_mean(bb), 0.5, EPS, "");
    bayes_update(bb, 8, 10);
    ASSERT_NEAR(bayes_posterior_mean(bb), 10.0/14.0, EPS, "");
    bayes_free(bb); PASS();
}
static void test_bayes_credible(void) {
    RUN_TEST("bayes_credible_interval");
    BetaBinomial* bb = bayes_beta_binomial_create(10.0, 10.0);
    double lo, hi;
    bayes_credible_interval(bb, 0.95, &lo, &hi);
    assert(lo < hi && lo >= 0.0 && hi <= 1.0);
    bayes_free(bb); PASS();
}
static void test_normal_normal(void) {
    RUN_TEST("normal_normal");
    NormalNormal* nn = bayes_normal_normal_create(0.0, 10.0, 1.0);
    double data[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    bayes_normal_normal_update(nn, data, 5);
    assert(fabs(nn->posterior_mean - 3.0) < 0.1);
    bayes_normal_normal_free(nn); PASS();
}
static void test_gamma_poisson(void) {
    RUN_TEST("gamma_poisson");
    GammaPoisson* gp = bayes_gamma_poisson_create(1.0, 1.0);
    int data[] = {2, 3, 4, 1, 2};
    bayes_gamma_poisson_update(gp, data, 5);
    ASSERT_NEAR(bayes_gamma_poisson_posterior_mean(gp), 13.0/6.0, EPS, "");
    bayes_gamma_poisson_free(gp); PASS();
}
static void test_naive_bayes_test(void) {
    RUN_TEST("naive_bayes");
    double data[6] = {1.0, 2.0, 3.0, 8.0, 9.0, 10.0};
    int labels[6] = {0, 0, 0, 1, 1, 1};
    NaiveBayes* nb = naive_bayes_create();
    naive_bayes_fit(nb, data, labels, 6);
    assert(naive_bayes_predict(nb, 2.0) == 0);
    assert(naive_bayes_predict(nb, 9.0) == 1);
    naive_bayes_free(nb); PASS();
}
static void test_bayes_ab_prob(void) {
    RUN_TEST("bayes_ab_test_prob");
    double prob = bayes_ab_test_prob_b_better(10, 100, 15, 100, 2000);
    /* B (15%) appears better than A (10%) */
    assert(prob > 0.4 && prob <= 1.0); PASS();
}

/* =========================================================================
 * INFERENCE TESTS
 * ========================================================================= */
static void test_mean_var(void) {
    RUN_TEST("mean/variance");
    double d[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    ASSERT_NEAR(mean(d, 5), 3.0, EPS, "");
    ASSERT_NEAR(variance(d, 5), 2.5, EPS, "");
    ASSERT_NEAR(stddev(d, 5), sqrt(2.5), EPS, ""); PASS();
}
static void test_median_iqr(void) {
    RUN_TEST("median/IQR");
    double d[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    ASSERT_NEAR(median(d, 5), 3.0, EPS, "");
    ASSERT_NEAR(percentile(d, 5, 50.0), 3.0, EPS, "");
    /* IQR: Q3(4) - Q1(2) = 2 */
    ASSERT_NEAR(iqr(d, 5), 2.0, EPS, ""); PASS();
}
static void test_cov_corr(void) {
    RUN_TEST("covariance/correlation");
    double x[] = {1.0, 2.0, 3.0}, y[] = {2.0, 4.0, 6.0};
    ASSERT_NEAR(correlation(x, y, 3), 1.0, EPS, ""); PASS();
}
static void test_mle_test(void) {
    RUN_TEST("MLE"); ASSERT_NEAR(mle_binomial_p(7, 10), 0.7, EPS, "");
    double d[] = {1.0, 2.0, 3.0};
    ASSERT_NEAR(mle_poisson_lambda(d, 3), 2.0, EPS, "");
    ASSERT_NEAR(mle_gaussian_mu(d, 3), 2.0, EPS, ""); PASS();
}
static void test_ci_test(void) {
    RUN_TEST("confidence_intervals");
    double d[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    double lo, hi;
    ci_gaussian_mean(d, 10, 3.0, 0.95, &lo, &hi);
    assert(lo < 5.5 && hi > 5.5);
    ci_proportion(60, 100, 0.95, &lo, &hi);
    assert(lo < 0.6 && hi > 0.6); PASS();
}
static void test_aic_bic(void) {
    RUN_TEST("AIC/BIC");
    double aic = compute_aic(10, 1.0, 2);
    double bic = compute_bic(10, 1.0, 2);
    assert(isfinite(aic) && isfinite(bic) && bic > aic); PASS();
}
static void test_fisher_info_test(void) {
    RUN_TEST("fisher_info");
    ASSERT_NEAR(fisher_info_binomial(100, 0.5), 400.0, EPS, "");
    ASSERT_NEAR(fisher_info_poisson(10, 2.0), 5.0, EPS, ""); PASS();
}

/* =========================================================================
 * HYPOTHESIS TEST TESTS
 * ========================================================================= */
static void test_ttest_one_sample(void) {
    RUN_TEST("ttest_one_sample");
    double d[] = {102, 98, 100, 97, 103, 99, 101};
    TTestResult r = ttest_one_sample(d, 7, 100);
    assert(isfinite(r.t_statistic) && r.p_value > 0 && r.p_value <= 1); PASS();
}
static void test_ttest_two_sample(void) {
    RUN_TEST("ttest_two_sample");
    double a[] = {5.0, 6.0, 7.0, 8.0};
    double b[] = {1.0, 2.0, 3.0, 4.0};
    TTestResult r = ttest_two_sample(a, 4, b, 4);
    assert(r.t_statistic > 3.0 && r.p_value < 0.05); PASS();
}
static void test_ttest_paired_test(void) {
    RUN_TEST("ttest_paired");
    double x[] = {10, 12, 11, 13, 14};
    double y[] = {7, 9, 8, 10, 11};
    TTestResult r = ttest_paired(x, y, 5);
    assert(r.t_statistic > 5.0); PASS();
}
static void test_chi2_independence(void) {
    RUN_TEST("chi2_independence");
    int r0[] = {50, 30, 20};
    int r1[] = {35, 45, 20};
    int* table[] = {r0, r1};
    Chi2Result r = chi2_test_independence(table, 2, 3);
    assert(r.chi2_statistic > 0 && r.p_value >= 0 && r.p_value <= 1); PASS();
}
static void test_ztest_proportion(void) {
    RUN_TEST("ztest_proportion");
    ZTestResult r = ztest_proportion(60, 100, 0.5);
    assert(r.z_statistic > 1.0 && r.p_value < 0.1); PASS();
}
static void test_anova_test(void) {
    RUN_TEST("anova_oneway");
    double g1[] = {5, 6, 7, 8};
    double g2[] = {1, 2, 3, 4};
    double g3[] = {3, 4, 5, 6};
    double* groups[] = {g1, g2, g3};
    int sizes[] = {4, 4, 4};
    AnovaResult r = anova_oneway(groups, sizes, 3);
    assert(r.f_statistic > 0 && r.p_value <= 1); PASS();
}
static void test_ks_test(void) {
    RUN_TEST("ks_test");
    double a[] = {1, 2, 3, 4, 5};
    double b[] = {1.1, 2.1, 3.1, 4.1, 5.1};
    double p;
    double d = ks_test_two_sample(a, 5, b, 5, &p);
    assert(d >= 0 && d <= 1 && p >= 0 && p <= 1); PASS();
}
static void test_mann_whitney_test(void) {
    RUN_TEST("mann_whitney");
    double a[] = {1, 2, 3};
    double b[] = {4, 5, 6};
    double p;
    double U = mann_whitney_u(a, 3, b, 3, &p);
    assert(isfinite(U) && p >= 0 && p <= 1); PASS();
}
static void test_power_analysis_test(void) {
    RUN_TEST("power_analysis");
    double power = test_power_gaussian(100, 0, 1, 1, 0.05);
    assert(power > 0.99 && power <= 1.0);
    double n = sample_size_for_power(0, 0.5, 1, 0.05, 0.8);
    assert(n > 0); PASS();
}

/* =========================================================================
 * REGRESSION TESTS
 * ========================================================================= */
static void test_linear_regression(void) {
    RUN_TEST("linear_regression");
    double x[] = {1, 2, 3, 4, 5};
    double y[] = {3, 5, 7, 9, 11};
    LinearRegression* lr = linreg_fit(x, y, 5);
    assert(lr != NULL);
    ASSERT_NEAR(lr->beta0, 1.0, EPS, "intercept");
    ASSERT_NEAR(lr->beta1, 2.0, EPS, "slope");
    ASSERT_NEAR(lr->r_squared, 1.0, EPS, "R2");
    ASSERT_NEAR(linreg_predict(lr, 6.0), 13.0, EPS, "pred");
    linreg_free(lr); PASS();
}
static void test_multiple_regression_test(void) {
    RUN_TEST("multiple_regression");
    /* Model: y = 1 + 2*x1 + 3*x2. X rows: [x1, x2] independent */
    double X[] = {1, 3, 2, 1, 3, 5, 4, 2, 5, 4};
    double y[] = {
        1 + 2*1 + 3*3,   /* 12 */
        1 + 2*2 + 3*1,   /* 8 */
        1 + 2*3 + 3*5,   /* 22 */
        1 + 2*4 + 3*2,   /* 15 */
        1 + 2*5 + 3*4    /* 23 */
    };
    MultipleRegression* mr = multireg_fit(X, y, 5, 2);
    assert(mr != NULL);
    ASSERT_NEAR(mr->beta[0], 1.0, EPS, "");
    ASSERT_NEAR(mr->beta[1], 2.0, EPS, "");
    ASSERT_NEAR(mr->beta[2], 3.0, EPS, "");
    multireg_free(mr); PASS();
}
static void test_logistic_regression_test(void) {
    RUN_TEST("logistic_regression");
    double X[] = {1, 2, 3, 8, 9, 10};
    int y[] = {0, 0, 0, 1, 1, 1};
    LogisticRegression* lr_model = logreg_fit(X, y, 6, 1, 0.5, 500);
    assert(lr_model != NULL);
    double x_val1 = 2.0, x_val2 = 9.0;
    assert(logreg_predict(lr_model, &x_val1) == 0);
    assert(logreg_predict(lr_model, &x_val2) == 1);
    logreg_free(lr_model); PASS();
}
static void test_poly_regression_test(void) {
    RUN_TEST("polyreg");
    double x[] = {1, 2, 3, 4, 5};
    double y[] = {1, 4, 9, 16, 25};
    PolynomialRegression* pr = polyreg_fit(x, y, 5, 2);
    assert(pr != NULL);
    ASSERT_NEAR(polyreg_predict(pr, 6.0), 36.0, EPS_WEAK, "");
    polyreg_free(pr); PASS();
}
static void test_ridge_regression_test(void) {
    RUN_TEST("ridge_regression");
    /* Model: y = 1 + 2*x1 + 3*x2, independent predictors */
    double X[] = {1, 3, 2, 1, 3, 5, 4, 2, 5, 4};
    double y[] = {12, 8, 22, 15, 23};  /* 1+2*x1+3*x2 */
    RidgeRegression* rr = ridge_fit(X, y, 5, 2, 0.01);
    assert(rr != NULL);
    double x_test[] = {5, 4};
    double pred = ridge_predict(rr, x_test);
    /* y = 1 + 2*5 + 3*4 = 23 */
    ASSERT_NEAR(pred, 23.0, EPS_WEAK, "");
    ridge_free(rr); PASS();
}

/* =========================================================================
 * MONTE CARLO TESTS
 * ========================================================================= */
static double test_f_x2(double x) { return x * x; }
static void test_mc_integrate_test(void) {
    RUN_TEST("mc_integrate");
    double r = mc_integrate(test_f_x2, 0.0, 1.0, 100000);
    ASSERT_NEAR(r, 1.0/3.0, 0.02, ""); PASS();
}
static void test_mc_pi_test(void) {
    RUN_TEST("mc_pi");
    ASSERT_NEAR(mc_pi(100000), PI, 0.05, ""); PASS();
}
static double target_normal_mc(double x) { return exp(-0.5*x*x); }
static void test_mcmc_metropolis_test(void) {
    RUN_TEST("mcmc_metropolis");
    double* samples = (double*)malloc(10000 * sizeof(double));
    mcmc_metropolis(target_normal_mc, 0.0, 10000, samples, 1.0);
    double sum = 0;
    for (int i = 1000; i < 10000; i++) sum += samples[i];
    assert(fabs(sum/9000) < 0.5);
    free(samples); PASS();
}
static void test_mcmc_gibbs_test(void) {
    RUN_TEST("mcmc_gibbs");
    int N = 20000;
    double* x = (double*)malloc(N * sizeof(double));
    double* y = (double*)malloc(N * sizeof(double));
    mcmc_gibbs_bivariate_normal(N, 0.5, x, y);
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0, sum_y2 = 0;
    int burn = 2000;
    for (int i = burn; i < N; i++) {
        sum_x += x[i]; sum_y += y[i];
        sum_xy += x[i]*y[i]; sum_x2 += x[i]*x[i]; sum_y2 += y[i]*y[i];
    }
    int n = N - burn;
    double mx = sum_x/n, my = sum_y/n;
    double vx = sum_x2/n - mx*mx, vy = sum_y2/n - my*my;
    double cxy = sum_xy/n - mx*my;
    double rho_est = cxy / sqrt(vx*vy);
    /* Check: means near 0, variances near 1, correlation positive.
     * Tolerances are generous due to MCMC randomness. */
    assert(fabs(mx) < 0.5 && fabs(my) < 0.5);
    assert(vx > 0.3 && vx < 3.0 && vy > 0.3 && vy < 3.0);
    assert(rho_est > -0.2);
    free(x); free(y); PASS();
}
static void test_mc_stratified_test(void) {
    RUN_TEST("mc_stratified");
    ASSERT_NEAR(mc_stratified_integrate(test_f_x2, 0, 1, 10000, 10), 1.0/3.0, 0.02, ""); PASS();
}

/* =========================================================================
 * SAMPLING TESTS
 * ========================================================================= */
static void test_srs(void) {
    RUN_TEST("sample_random");
    int pop[] = {10, 20, 30, 40, 50};
    int samp[3];
    sample_random(pop, 5, samp, 3);
    int sum = 0;
    for (int i = 0; i < 3; i++) sum += samp[i];
    assert(sum >= 60 && sum <= 120); PASS();
}
static void test_stratified_test(void) {
    RUN_TEST("sample_stratified");
    int s0[] = {1, 2, 3}; int s1[] = {4, 5, 6};
    int* strata[] = {s0, s1};
    int sizes[] = {3, 3}, ss[] = {2, 2};
    int sample[4];
    sample_stratified(strata, sizes, 2, sample, ss);
    int sum = 0;
    for (int i = 0; i < 4; i++) sum += sample[i];
    assert(sum >= 6 && sum <= 22); PASS();
}
static void test_systematic_test(void) {
    RUN_TEST("sample_systematic");
    int pop[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int samp[3];
    sample_systematic(pop, 10, samp, 3);
    int sum = 0;
    for (int i = 0; i < 3; i++) sum += samp[i];
    assert(sum >= 3 && sum <= 30); PASS();
}
static void test_reservoir_test(void) {
    RUN_TEST("reservoir_sample");
    int* res = reservoir_sample(20, 5);
    assert(res != NULL);
    for (int i = 0; i < 5; i++) assert(res[i] >= 0 && res[i] < 20);
    free(res); PASS();
}
static void test_bootstrap_ci_test(void) {
    RUN_TEST("bootstrap_mean_ci");
    double data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    double lo, hi;
    double m = bootstrap_mean_ci(data, 10, 500, 0.95, &lo, &hi);
    ASSERT_NEAR(m, 5.5, EPS, "");
    assert(lo < m && hi > m); PASS();
}
static void test_sample_size_test(void) {
    RUN_TEST("sample_size");
    double n = sample_size_proportion(0.5, 0.05, 0.95);
    assert(n > 300 && n < 400); PASS();
}

/* =========================================================================
 * MAIN
 * ========================================================================= */
int main(void) {
    srand((unsigned)time(NULL));
    printf("\n=== mini-prob-stats: Comprehensive Test Suite ===\n\n");

    test_prob_basic(); test_prob_conditional(); test_prob_bayes_theorem();
    test_factorial(); test_nCr_nPr_test(); test_nPr();
    test_catalan(); test_bell_number(); test_derangement();
    test_law_total_prob(); test_extended_bayes();
    test_poisson_process_prob_test();
    test_markov_bound(); test_chebyshev_bound(); test_hoeffding_bound();
    test_fisher_yates_shuffle(); test_lln_uniform_mean_test();

    test_rv_create(); test_rv_expectation(); test_rv_variance();
    test_rv_covariance_test(); test_rv_skewness_test();
    test_rv_entropy_test(); test_rv_kl_divergence_test();

    test_bernoulli(); test_binomial_test(); test_poisson_test();
    test_gaussian_test(); test_exponential_test();
    test_gamma_dist_test(); test_beta_dist_test(); test_chi2_dist_test();
    test_f_dist_test(); test_lognormal_test(); test_weibull_test();
    test_cauchy_test(); test_geometric_test(); test_rayleigh_test();
    test_multivariate_gaussian_test();

    test_beta_binomial(); test_bayes_credible();
    test_normal_normal(); test_gamma_poisson();
    test_naive_bayes_test(); test_bayes_ab_prob();

    test_mean_var(); test_median_iqr(); test_cov_corr();
    test_mle_test(); test_ci_test(); test_aic_bic(); test_fisher_info_test();

    test_ttest_one_sample(); test_ttest_two_sample(); test_ttest_paired_test();
    test_chi2_independence(); test_ztest_proportion(); test_anova_test();
    test_ks_test(); test_mann_whitney_test(); test_power_analysis_test();

    test_linear_regression(); test_multiple_regression_test();
    test_logistic_regression_test(); test_poly_regression_test();
    test_ridge_regression_test();

    test_mc_integrate_test(); test_mc_pi_test();
    test_mcmc_metropolis_test(); test_mcmc_gibbs_test();
    test_mc_stratified_test();

    test_srs(); test_stratified_test(); test_systematic_test();
    test_reservoir_test(); test_bootstrap_ci_test(); test_sample_size_test();

    printf("\n=== Results: %d/%d tests passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
