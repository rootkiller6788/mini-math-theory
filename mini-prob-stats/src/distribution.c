#include "distribution.h"
#include "probability.h"
#include <math.h>
#include <stdlib.h>

double bernoulli_pmf(int k, double p) {
    if (k == 1) return p;
    if (k == 0) return 1.0 - p;
    return 0.0;
}

double bernoulli_expectation(double p) {
    return p;
}

double bernoulli_variance(double p) {
    return p * (1.0 - p);
}

double binomial_pmf(int k, int n, double p) {
    if (k < 0 || k > n) return 0.0;
    return (double)nCr(n, k) * pow(p, k) * pow(1.0 - p, n - k);
}

double binomial_expectation(int n, double p) {
    return n * p;
}

double binomial_variance(int n, double p) {
    return n * p * (1.0 - p);
}

double poisson_pmf(int k, double lambda) {
    if (k < 0 || lambda < 0.0) return 0.0;
    return pow(lambda, k) * exp(-lambda) / (double)factorial(k);
}

double poisson_expectation(double lambda) {
    return lambda;
}

double poisson_variance(double lambda) {
    return lambda;
}

double gaussian_pdf(double x, double mu, double sigma) {
    double z = (x - mu) / sigma;
    return exp(-0.5 * z * z) / (sigma * sqrt(2.0 * PI));
}

double gaussian_cdf(double x, double mu, double sigma) {
    return 0.5 * (1.0 + erf((x - mu) / (sigma * sqrt(2.0))));
}

double gaussian_expectation(double mu, double sigma) {
    (void)sigma;
    return mu;
}

double gaussian_variance(double mu, double sigma) {
    (void)mu;
    return sigma * sigma;
}

double gaussian_quantile(double p) {
    if (p <= 0.0) return -1e10;
    if (p >= 1.0) return 1e10;

    double a0 = -3.969683028665376e+01;
    double a1 =  2.209460984245205e+02;
    double a2 = -2.759285104469687e+02;
    double a3 =  1.383577518672690e+02;
    double a4 = -3.066479806614716e+01;
    double a5 =  2.506628277459239e+00;

    double b0 = -5.447609879822406e+01;
    double b1 =  1.615858368580409e+02;
    double b2 = -1.556989798598866e+02;
    double b3 =  6.680131188771972e+01;
    double b4 = -1.328068155288572e+01;

    double c0 = -7.784894002430293e-03;
    double c1 = -3.223964580411365e-01;
    double c2 = -2.400758277161838e+00;
    double c3 = -2.549732539343734e+00;
    double c4 =  4.374664141464968e+00;
    double c5 =  2.938163982698783e+00;

    double d0 =  7.784695709041462e-03;
    double d1 =  3.224671290700398e-01;
    double d2 =  2.445134137142996e+00;
    double d3 =  3.754408661907416e+00;

    double p_low  = 0.02425;
    double p_high = 1.0 - p_low;
    double q, r, x;

    if (p < p_low) {
        q = sqrt(-2.0 * log(p));
        x = (((((c0 * q + c1) * q + c2) * q + c3) * q + c4) * q + c5)
          / ((((d0 * q + d1) * q + d2) * q + d3) * q + 1.0);
    } else if (p <= p_high) {
        q = p - 0.5;
        r = q * q;
        x = (((((a0 * r + a1) * r + a2) * r + a3) * r + a4) * r + a5) * q
          / (((((b0 * r + b1) * r + b2) * r + b3) * r + b4) * r + 1.0);
    } else {
        q = sqrt(-2.0 * log(1.0 - p));
        x = -((((((c0 * q + c1) * q + c2) * q + c3) * q + c4) * q + c5)
            / ((((d0 * q + d1) * q + d2) * q + d3) * q + 1.0));
    }
    return x;
}

double uniform_pdf(double x, double a, double b) {
    if (x < a || x > b) return 0.0;
    return 1.0 / (b - a);
}

double uniform_expectation(double a, double b) {
    return (a + b) / 2.0;
}

double uniform_variance(double a, double b) {
    double w = b - a;
    return (w * w) / 12.0;
}

double exponential_pdf(double x, double lambda) {
    if (x < 0.0) return 0.0;
    return lambda * exp(-lambda * x);
}

double exponential_cdf(double x, double lambda) {
    if (x < 0.0) return 0.0;
    return 1.0 - exp(-lambda * x);
}

double exponential_expectation(double lambda) {
    if (lambda <= 0.0) return 0.0;
    return 1.0 / lambda;
}

double exponential_variance(double lambda) {
    if (lambda <= 0.0) return 0.0;
    return 1.0 / (lambda * lambda);
}

double* sample_mean_distribution(int sample_size, int n_samples) {
    double* means = (double*)malloc(n_samples * sizeof(double));
    if (!means) return NULL;
    for (int i = 0; i < n_samples; i++) {
        double sum = 0.0;
        for (int j = 0; j < sample_size; j++) {
            sum += (double)rand() / RAND_MAX;
        }
        means[i] = sum / sample_size;
    }
    return means;
}
