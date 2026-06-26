#ifndef RANDOM_VARIABLE_H
#define RANDOM_VARIABLE_H

/* L1: Discrete Random Variable struct */
typedef struct {
    double* values;
    double* probs;
    int n;
} DiscreteRV;

/* L1: Lifecycle */
DiscreteRV* rv_create(int n);
void rv_free(DiscreteRV* rv);
void rv_set(DiscreteRV* rv, int i, double value, double prob);
void rv_normalize(DiscreteRV* rv);

/* L2: Core statistics */
double rv_expectation(DiscreteRV* rv);
double rv_expectation_func(DiscreteRV* rv, double (*f)(double));
double rv_variance(DiscreteRV* rv);
double rv_stddev(DiscreteRV* rv);

/* L2: Moments */
double rv_moment(DiscreteRV* rv, int k);
double rv_central_moment(DiscreteRV* rv, int k);
double rv_skewness(DiscreteRV* rv);
double rv_excess_kurtosis(DiscreteRV* rv);

/* L2: Joint distributions */
double rv_covariance(DiscreteRV* x, DiscreteRV* y);
double rv_correlation(DiscreteRV* x, DiscreteRV* y);
void rv_joint_independent(DiscreteRV* x, DiscreteRV* y,
                           double* joint_values, double* joint_probs, int* n_out);
double rv_conditional_expectation(DiscreteRV* joint, double x_val,
                                   int n_y, double* y_vals);

/* L4: Inequality verification */
double rv_chebyshev_verify(DiscreteRV* rv, double k);
double rv_markov_verify(DiscreteRV* rv, double a);

/* L5: Generating functions */
double rv_mgf(DiscreteRV* rv, double t);
double rv_pgf(DiscreteRV* rv, double z);

/* L5: Information theory */
double rv_entropy(DiscreteRV* rv);
double rv_kl_divergence(DiscreteRV* p, DiscreteRV* q);

/* L2: Continuous RV (numerical) */
double crv_expectation(double (*pdf)(double), double a, double b, int n_grid);
double crv_variance(double (*pdf)(double), double a, double b, int n_grid);
double crv_cdf(double (*pdf)(double), double x, double a, double b, int n_grid);

void rv_print(DiscreteRV* rv);

#endif
