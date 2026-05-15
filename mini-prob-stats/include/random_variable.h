#ifndef RANDOM_VARIABLE_H
#define RANDOM_VARIABLE_H

typedef struct {
    double* values;
    double* probs;
    int n;
} DiscreteRV;

DiscreteRV* rv_create(int n);
void rv_free(DiscreteRV* rv);
void rv_set(DiscreteRV* rv, int i, double value, double prob);
double rv_expectation(DiscreteRV* rv);
double rv_variance(DiscreteRV* rv);
double rv_stddev(DiscreteRV* rv);
double rv_expectation_func(DiscreteRV* rv, double (*f)(double));
double rv_covariance(DiscreteRV* x, DiscreteRV* y);
double rv_correlation(DiscreteRV* x, DiscreteRV* y);
void rv_print(DiscreteRV* rv);

#endif
