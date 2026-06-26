/* random_variable.c — Discrete and continuous random variables.
 *
 * L1: DiscreteRV with PMF, expectation, variance, moments
 * L2: ContinuousRV approximations via numerical integration
 * L4: Chebyshev / Markov inequality verification
 * L5: Joint distributions, conditional expectation
 *
 * Theorem sources:
 *   MIT 18.05 §3 (Random Variables)
 *   Ross §4–§7 (Expectation, Joint Distributions)
 */

#include "random_variable.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

static double square(double x) { return x * x; }

/* --------------------------------------------------------------------------
 * L1: Discrete Random Variable
 * -------------------------------------------------------------------------- */
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

/* L2: Normalize probabilities to sum to 1 */
void rv_normalize(DiscreteRV* rv) {
    if (!rv) return;
    double sum = 0.0;
    for (int i = 0; i < rv->n; i++) sum += rv->probs[i];
    if (sum <= 0.0) return;
    for (int i = 0; i < rv->n; i++) rv->probs[i] /= sum;
}

double rv_expectation(DiscreteRV* rv) {
    if (!rv) return 0.0;
    double e = 0.0;
    for (int i = 0; i < rv->n; i++) {
        e += rv->values[i] * rv->probs[i];
    }
    return e;
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

/* L2: Moments μ_k = E[X^k], central moments ν_k = E[(X-μ)^k] */
double rv_moment(DiscreteRV* rv, int k) {
    if (!rv || k < 1) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < rv->n; i++) {
        sum += pow(rv->values[i], k) * rv->probs[i];
    }
    return sum;
}

double rv_central_moment(DiscreteRV* rv, int k) {
    if (!rv || k < 2) return 0.0;
    double mu = rv_expectation(rv);
    double sum = 0.0;
    for (int i = 0; i < rv->n; i++) {
        sum += pow(rv->values[i] - mu, k) * rv->probs[i];
    }
    return sum;
}

/* L2: Skewness γ₁ = μ₃/σ³  (asymmetry, > 0 means right-skewed) */
double rv_skewness(DiscreteRV* rv) {
    double sd = rv_stddev(rv);
    if (sd == 0.0) return 0.0;
    return rv_central_moment(rv, 3) / (sd * sd * sd);
}

/* L2: Excess kurtosis κ = μ₄/σ⁴ - 3 (Normal has excess=0) */
double rv_excess_kurtosis(DiscreteRV* rv) {
    double var = rv_variance(rv);
    if (var == 0.0) return 0.0;
    return rv_central_moment(rv, 4) / (var * var) - 3.0;
}

/* L2: Covariance and correlation */
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

/* --------------------------------------------------------------------------
 * L2: Continuous Random Variable (numerical integration)
 *
 * Represented by a PDF function and integration bounds.
 * All expectations computed via trapezoidal rule over grid.
 * -------------------------------------------------------------------------- */
double crv_expectation(double (*pdf)(double), double a, double b, int n_grid) {
    if (n_grid < 2) return 0.0;
    double dx = (b - a) / (n_grid - 1);
    double sum = 0.0;
    for (int i = 0; i < n_grid; i++) {
        double x = a + i * dx;
        double w = (i == 0 || i == n_grid - 1) ? 0.5 : 1.0;
        sum += w * x * pdf(x) * dx;
    }
    return sum;
}

double crv_variance(double (*pdf)(double), double a, double b, int n_grid) {
    double mu = crv_expectation(pdf, a, b, n_grid);
    double dx = (b - a) / (n_grid - 1);
    double sum = 0.0;
    for (int i = 0; i < n_grid; i++) {
        double x = a + i * dx;
        double w = (i == 0 || i == n_grid - 1) ? 0.5 : 1.0;
        double d = x - mu;
        sum += w * d * d * pdf(x) * dx;
    }
    return sum;
}

double crv_cdf(double (*pdf)(double), double x, double a, double b, int n_grid) {
    if (x < a) return 0.0;
    if (x >= b) return 1.0;
    double dx = (x - a) / (n_grid - 1);
    double sum = 0.0;
    for (int i = 0; i < n_grid; i++) {
        double xi = a + i * dx;
        double w = (i == 0 || i == n_grid - 1) ? 0.5 : 1.0;
        sum += w * pdf(xi) * dx;
    }
    return sum;
}

/* L5: Conditional expectation E[Y|X=x] for discrete joint RV */
double rv_conditional_expectation(DiscreteRV* joint, double x_val,
                                   int n_y, double* y_vals) {
    if (!joint || n_y <= 0 || !y_vals) return 0.0;
    double prob_x = 0.0, weighted_sum = 0.0;
    for (int i = 0; i < joint->n; i++) {
        if (fabs(joint->values[i] - x_val) < 1e-10) {
            prob_x += joint->probs[i];
        }
    }
    if (prob_x <= 0.0) return 0.0;
    for (int i = 0; i < joint->n; i++) {
        if (fabs(joint->values[i] - x_val) < 1e-10) {
            /* y_vals should be paired by index, simplified here */
            weighted_sum += y_vals[i % n_y] * joint->probs[i];
        }
    }
    return weighted_sum / prob_x;
}

/* L4: Chebyshev inequality verification
 * P(|X-μ| ≥ kσ) ≤ 1/k²
 * Returns actual proportion exceeding k SD from mean.
 */
double rv_chebyshev_verify(DiscreteRV* rv, double k) {
    if (!rv) return 1.0;
    double mu = rv_expectation(rv);
    double sigma = rv_stddev(rv);
    if (sigma == 0.0) return 0.0;
    double threshold = k * sigma;
    double prob = 0.0;
    for (int i = 0; i < rv->n; i++) {
        if (fabs(rv->values[i] - mu) >= threshold) {
            prob += rv->probs[i];
        }
    }
    return prob;
}

/* L4: Markov inequality verification
 * P(X ≥ a) ≤ E[X]/a for X ≥ 0
 */
double rv_markov_verify(DiscreteRV* rv, double a) {
    if (!rv || a <= 0.0) return 1.0;
    double prob = 0.0;
    for (int i = 0; i < rv->n; i++) {
        if (rv->values[i] >= a) prob += rv->probs[i];
    }
    return prob;
}

/* L2: Joint distribution — product of independent RVs */
void rv_joint_independent(DiscreteRV* x, DiscreteRV* y,
                           double* joint_values, double* joint_probs, int* n_out) {
    if (!x || !y || !n_out) return;
    int count = 0;
    for (int i = 0; i < x->n; i++) {
        for (int j = 0; j < y->n; j++) {
            if (joint_values) joint_values[count] = x->values[i] * y->values[j];
            if (joint_probs) joint_probs[count] = x->probs[i] * y->probs[j];
            count++;
        }
    }
    *n_out = count;
}

/* L5: Moment Generating Function M(t) = E[e^{tX}] for discrete RV */
double rv_mgf(DiscreteRV* rv, double t) {
    if (!rv) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < rv->n; i++) {
        sum += exp(t * rv->values[i]) * rv->probs[i];
    }
    return sum;
}

/* L5: Probability Generating Function G(z) = E[z^X] for non-negative integer RV */
double rv_pgf(DiscreteRV* rv, double z) {
    if (!rv) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < rv->n; i++) {
        sum += pow(z, rv->values[i]) * rv->probs[i];
    }
    return sum;
}

/* L5: Entropy H(X) = -Σ pᵢ log(pᵢ) of discrete RV */
double rv_entropy(DiscreteRV* rv) {
    if (!rv) return 0.0;
    double h = 0.0;
    for (int i = 0; i < rv->n; i++) {
        if (rv->probs[i] > 0.0) {
            h -= rv->probs[i] * log(rv->probs[i]);
        }
    }
    return h;
}

/* L5: KL divergence D_KL(P||Q) = Σ pᵢ log(pᵢ/qᵢ) */
double rv_kl_divergence(DiscreteRV* p, DiscreteRV* q) {
    if (!p || !q || p->n != q->n) return 0.0;
    double kl = 0.0;
    for (int i = 0; i < p->n; i++) {
        if (p->probs[i] > 0.0 && q->probs[i] > 0.0) {
            kl += p->probs[i] * log(p->probs[i] / q->probs[i]);
        } else if (p->probs[i] > 0.0) {
            return 1.0 / 0.0;  /* infinite */
        }
    }
    return kl;
}
