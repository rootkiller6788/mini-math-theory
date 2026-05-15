#include "kl_divergence.h"
#include <math.h>
#include <stdlib.h>

double kl_divergence(const double* p, const double* q, int n) {
    double d = 0.0;
    for (int i = 0; i < n; i++)
        if (p[i] > 0.0 && q[i] > 0.0)
            d += p[i] * log2(p[i] / q[i]);
    return d;
}

double cross_entropy(const double* p, const double* q, int n) {
    double h = 0.0;
    for (int i = 0; i < n; i++)
        if (q[i] > 0.0)
            h -= p[i] * log2(q[i]);
    return h;
}

double js_divergence(const double* p, const double* q, int n) {
    double* m = malloc(n * sizeof(double));
    for (int i = 0; i < n; i++)
        m[i] = (p[i] + q[i]) / 2.0;

    double js = 0.5 * kl_divergence(p, m, n) + 0.5 * kl_divergence(q, m, n);
    free(m);
    return js;
}

double kl_divergence_smoothed(const double* p, const double* q, int n, double eps) {
    double* ps = malloc(n * sizeof(double));
    double* qs = malloc(n * sizeof(double));
    double np = 0, nq = 0;
    for (int i = 0; i < n; i++) {
        ps[i] = p[i] + eps;
        np += ps[i];
        qs[i] = q[i] + eps;
        nq += qs[i];
    }
    for (int i = 0; i < n; i++) { ps[i] /= np; qs[i] /= nq; }
    double d = kl_divergence(ps, qs, n);
    free(ps); free(qs);
    return d;
}

double perplexity(const double* p, const double* q, int n) {
    double ce = cross_entropy(p, q, n);
    return pow(2.0, ce);
}
