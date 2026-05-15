#ifndef KL_DIVERGENCE_H
#define KL_DIVERGENCE_H

double kl_divergence(const double* p, const double* q, int n);
double cross_entropy(const double* p, const double* q, int n);
double js_divergence(const double* p, const double* q, int n);
double kl_divergence_smoothed(const double* p, const double* q, int n, double eps);
double perplexity(const double* p, const double* q, int n);

#endif
