#ifndef INFERENCE_H
#define INFERENCE_H

double mle_binomial_p(int k, int n);
double mle_poisson_lambda(double* data, int n);
double mle_gaussian_mu(double* data, int n);
double mle_gaussian_sigma(double* data, int n);

void ci_gaussian_mean(double* data, int n, double sigma, double confidence,
                      double* lo, double* hi);
void ci_proportion(int k, int n, double confidence, double* lo, double* hi);
void ci_diff_means(double* d1, int n1, double* d2, int n2, double confidence,
                   double* lo, double* hi);

double mean(double* data, int n);
double variance(double* data, int n);
double stddev(double* data, int n);
double median(double* data, int n);
double percentile(double* data, int n, double p);
double covariance(double* x, double* y, int n);
double correlation(double* x, double* y, int n);

#endif
