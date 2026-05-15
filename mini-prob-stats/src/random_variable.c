#include "random_variable.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

DiscreteRV* rv_create(int n) {
    DiscreteRV* rv = (DiscreteRV*)malloc(sizeof(DiscreteRV));
    if (!rv) return NULL;
    rv->n = n;
    rv->values = (double*)malloc(n * sizeof(double));
    rv->probs = (double*)malloc(n * sizeof(double));
    if (!rv->values || !rv->probs) {
        free(rv->values);
        free(rv->probs);
        free(rv);
        return NULL;
    }
    for (int i = 0; i < n; i++) {
        rv->values[i] = 0.0;
        rv->probs[i] = 0.0;
    }
    return rv;
}

void rv_free(DiscreteRV* rv) {
    if (!rv) return;
    free(rv->values);
    free(rv->probs);
    free(rv);
}

void rv_set(DiscreteRV* rv, int i, double value, double prob) {
    if (!rv || i < 0 || i >= rv->n) return;
    rv->values[i] = value;
    rv->probs[i] = prob;
}

double rv_expectation(DiscreteRV* rv) {
    if (!rv) return 0.0;
    double e = 0.0;
    for (int i = 0; i < rv->n; i++) {
        e += rv->values[i] * rv->probs[i];
    }
    return e;
}

static double square(double x) {
    return x * x;
}

double rv_expectation_func(DiscreteRV* rv, double (*f)(double)) {
    if (!rv) return 0.0;
    double e = 0.0;
    for (int i = 0; i < rv->n; i++) {
        e += f(rv->values[i]) * rv->probs[i];
    }
    return e;
}

double rv_variance(DiscreteRV* rv) {
    if (!rv) return 0.0;
    double ex = rv_expectation(rv);
    double ex2 = rv_expectation_func(rv, square);
    return ex2 - ex * ex;
}

double rv_stddev(DiscreteRV* rv) {
    return sqrt(rv_variance(rv));
}

double rv_covariance(DiscreteRV* x, DiscreteRV* y) {
    if (!x || !y || x->n != y->n) return 0.0;
    double ex = rv_expectation(x);
    double ey = rv_expectation(y);
    double exy = 0.0;
    for (int i = 0; i < x->n; i++) {
        exy += x->values[i] * y->values[i] * x->probs[i];
    }
    return exy - ex * ey;
}

double rv_correlation(DiscreteRV* x, DiscreteRV* y) {
    double cov = rv_covariance(x, y);
    double sx = rv_stddev(x);
    double sy = rv_stddev(y);
    if (sx == 0.0 || sy == 0.0) return 0.0;
    return cov / (sx * sy);
}

void rv_print(DiscreteRV* rv) {
    if (!rv) return;
    printf("Discrete Random Variable:\n");
    for (int i = 0; i < rv->n; i++) {
        printf("  x=%.4f, P=%.4f\n", rv->values[i], rv->probs[i]);
    }
    printf("  E[X]=%.6f, Var(X)=%.6f\n", rv_expectation(rv), rv_variance(rv));
}
