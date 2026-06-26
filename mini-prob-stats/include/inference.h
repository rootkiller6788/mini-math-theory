#ifndef INFERENCE_H
#define INFERENCE_H

/* L1: Descriptive statistics */
double mean(double* data, int n);
double variance(double* data, int n);
double stddev(double* data, int n);
double median(double* data, int n);
double percentile(double* data, int n, double p);
double iqr(double* data, int n);
double mad(double* data, int n);
double covariance(double* x, double* y, int n);
double correlation(double* x, double* y, int n);

/* L4: MLE (Maximum Likelihood Estimation) */
double mle_binomial_p(int k, int n);
double mle_poisson_lambda(double* data, int n);
double mle_gaussian_mu(double* data, int n);
double mle_gaussian_sigma(double* data, int n);
double mle_exponential_lambda(double* data, int n);

/* L4: Confidence Intervals */
void ci_gaussian_mean(double* data, int n, double sigma, double confidence,
                      double* lo, double* hi);
void ci_proportion(int k, int n, double confidence, double* lo, double* hi);
void ci_diff_means(double* d1, int n1, double* d2, int n2, double confidence,
                   double* lo, double* hi);
void ci_variance(double* data, int n, double confidence, double* lo, double* hi);

/* L5: Bootstrap */
double* bootstrap_resample(double* data, int n);
void bootstrap_ci(double* data, int n, int n_bootstrap, double confidence,
                  double* lo, double* hi);

/* L5: EM Algorithm — Gaussian Mixture Model (2 components) */
void em_gaussian_mixture_1d(double* data, int n,
                            double* pi, double* mu, double* sigma,
                            int max_iter, double tol);

/* L5: Kernel Density Estimation */
double kde_bandwidth_silverman(double* data, int n);
double kde_evaluate(double x, double* data, int n, double bandwidth);
void kde_grid(double* data, int n, double bandwidth,
              double* grid_x, double* grid_density, int grid_n,
              double x_min, double x_max);

/* L5: AIC/BIC model selection */
double compute_aic(int n, double rss, int k_params);
double compute_bic(int n, double rss, int k_params);

/* L5: Fisher Information */
double fisher_info_binomial(int n, double p);
double fisher_info_poisson(int n, double lambda);
double fisher_info_gaussian_mu(int n, double sigma);

/* L5: Likelihood Ratio Test */
double likelihood_ratio_statistic(double* data, int n, double mu0, double sigma);

/* L6: Jackknife — leave-one-out estimation */
double jackknife_estimate(double* data, int n, double (*estimator)(double*, int));
double jackknife_variance(double* data, int n, double (*estimator)(double*, int));

#endif
