#include "inference.h"
#include "distribution.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

double mean(double* data, int n) {
    if (n <= 0) return 0.0;
    double s = 0.0;
    for (int i = 0; i < n; i++) s += data[i];
    return s / n;
}

double variance(double* data, int n) {
    if (n <= 1) return 0.0;
    double m = mean(data, n);
    double ss = 0.0;
    for (int i = 0; i < n; i++) {
        double d = data[i] - m;
        ss += d * d;
    }
    return ss / (n - 1);
}

double stddev(double* data, int n) {
    return sqrt(variance(data, n));
}

static int cmp_double(const void* a, const void* b) {
    double da = *(const double*)a;
    double db = *(const double*)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

double median(double* data, int n) {
    if (n <= 0) return 0.0;
    double* sorted = (double*)malloc(n * sizeof(double));
    if (!sorted) return 0.0;
    memcpy(sorted, data, n * sizeof(double));
    qsort(sorted, n, sizeof(double), cmp_double);
    double m;
    if (n % 2 == 0) {
        m = (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0;
    } else {
        m = sorted[n / 2];
    }
    free(sorted);
    return m;
}

double percentile(double* data, int n, double p) {
    if (n <= 0) return 0.0;
    if (p < 0.0) p = 0.0;
    if (p > 100.0) p = 100.0;
    double* sorted = (double*)malloc(n * sizeof(double));
    if (!sorted) return 0.0;
    memcpy(sorted, data, n * sizeof(double));
    qsort(sorted, n, sizeof(double), cmp_double);
    double rank = (p / 100.0) * (n - 1);
    int lo = (int)rank;
    double frac = rank - lo;
    double result;
    if (lo >= n - 1) {
        result = sorted[n - 1];
    } else {
        result = sorted[lo] + frac * (sorted[lo + 1] - sorted[lo]);
    }
    free(sorted);
    return result;
}

double covariance(double* x, double* y, int n) {
    if (n <= 1) return 0.0;
    double mx = mean(x, n);
    double my = mean(y, n);
    double ss = 0.0;
    for (int i = 0; i < n; i++) {
        ss += (x[i] - mx) * (y[i] - my);
    }
    return ss / (n - 1);
}

double correlation(double* x, double* y, int n) {
    double cov = covariance(x, y, n);
    double sx = stddev(x, n);
    double sy = stddev(y, n);
    if (sx == 0.0 || sy == 0.0) return 0.0;
    return cov / (sx * sy);
}

double mle_binomial_p(int k, int n) {
    if (n <= 0) return 0.0;
    return (double)k / n;
}

double mle_poisson_lambda(double* data, int n) {
    return mean(data, n);
}

double mle_gaussian_mu(double* data, int n) {
    return mean(data, n);
}

double mle_gaussian_sigma(double* data, int n) {
    if (n <= 0) return 0.0;
    double m = mean(data, n);
    double ss = 0.0;
    for (int i = 0; i < n; i++) {
        double d = data[i] - m;
        ss += d * d;
    }
    return sqrt(ss / n);
}

static double z_for_confidence(double confidence) {
    double tail = (1.0 - confidence) / 2.0;
    return gaussian_quantile(1.0 - tail);
}

void ci_gaussian_mean(double* data, int n, double sigma, double confidence,
                      double* lo, double* hi) {
    if (n <= 0) { *lo = *hi = 0.0; return; }
    double xbar = mean(data, n);
    double z = z_for_confidence(confidence);
    double margin = z * sigma / sqrt((double)n);
    *lo = xbar - margin;
    *hi = xbar + margin;
}

void ci_proportion(int k, int n, double confidence, double* lo, double* hi) {
    if (n <= 0) { *lo = *hi = 0.0; return; }
    double p_hat = (double)k / n;
    double z = z_for_confidence(confidence);
    double margin = z * sqrt(p_hat * (1.0 - p_hat) / n);
    *lo = p_hat - margin;
    *hi = p_hat + margin;
    if (*lo < 0.0) *lo = 0.0;
    if (*hi > 1.0) *hi = 1.0;
}

void ci_diff_means(double* d1, int n1, double* d2, int n2, double confidence,
                   double* lo, double* hi) {
    if (n1 <= 0 || n2 <= 0) { *lo = *hi = 0.0; return; }
    double xbar1 = mean(d1, n1);
    double xbar2 = mean(d2, n2);
    double v1 = variance(d1, n1);
    double v2 = variance(d2, n2);
    double z = z_for_confidence(confidence);
    double se = sqrt(v1 / n1 + v2 / n2);
    double diff = xbar1 - xbar2;
    double margin = z * se;
    *lo = diff - margin;
    *hi = diff + margin;
}
