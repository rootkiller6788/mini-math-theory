#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#define PI 3.14159265358979323846

/* Log-Gamma (Lanczos approximation) — used by multiple modules */
double log_gamma_stirling(double x);

/* L1: Bernoulli — k ∈ {0,1} */
double bernoulli_pmf(int k, double p);
double bernoulli_expectation(double p);
double bernoulli_variance(double p);

/* L1: Binomial — counts in n trials */
double binomial_pmf(int k, int n, double p);
double binomial_cdf(int k, int n, double p);
double binomial_expectation(int n, double p);
double binomial_variance(int n, double p);

/* L1: Poisson — counting rare events */
double poisson_pmf(int k, double lambda);
double poisson_cdf(int k, double lambda);
double poisson_expectation(double lambda);
double poisson_variance(double lambda);

/* L1: Gaussian (Normal) — fundamental to all statistics */
double gaussian_pdf(double x, double mu, double sigma);
double gaussian_cdf(double x, double mu, double sigma);
double gaussian_expectation(double mu, double sigma);
double gaussian_variance(double mu, double sigma);
double gaussian_quantile(double p);
double gaussian_skewness(void);
double gaussian_excess_kurtosis(void);

/* L1: Uniform — on [a,b] */
double uniform_pdf(double x, double a, double b);
double uniform_cdf(double x, double a, double b);
double uniform_expectation(double a, double b);
double uniform_variance(double a, double b);

/* L1: Exponential — memoryless */
double exponential_pdf(double x, double lambda);
double exponential_cdf(double x, double lambda);
double exponential_expectation(double lambda);
double exponential_variance(double lambda);
double exponential_skewness(void);
double exponential_excess_kurtosis(void);

/* L2: Gamma — wait time, generalized factorial */
double gamma_pdf(double x, double alpha, double beta);
double gamma_expectation(double alpha, double beta);
double gamma_variance(double alpha, double beta);

/* L2: Beta — on [0,1], conjugate prior for binomial */
double beta_pdf(double x, double alpha, double beta);
double beta_expectation(double alpha, double beta);
double beta_variance(double alpha, double beta);

/* L2: Student's t — robust, heavy-tailed (ν df) */
double t_pdf(double x, double nu);
double t_cdf(double x, double nu);
double t_expectation(double nu);
double t_variance(double nu);

/* L2: Chi-squared — sum of squared normals (k df) */
double chi2_pdf(double x, double k);
double chi2_expectation(double k);
double chi2_variance(double k);

/* L2: F — ratio of chi-squared */
double f_pdf(double x, double d1, double d2);
double f_expectation(double d1, double d2);
double f_variance(double d1, double d2);

/* L2: Lognormal — multiplicative processes */
double lognormal_pdf(double x, double mu, double sigma);
double lognormal_expectation(double mu, double sigma);
double lognormal_variance(double mu, double sigma);

/* L2: Weibull — flexible lifetime/ reliability */
double weibull_pdf(double x, double shape, double scale);
double weibull_cdf(double x, double shape, double scale);
double weibull_expectation(double shape, double scale);
double weibull_variance(double shape, double scale);

/* L2: Cauchy — heavy tails, undefined moments */
double cauchy_pdf(double x, double x0, double gamma);
double cauchy_cdf(double x, double x0, double gamma);

/* L4: CLT — sample mean distribution */
double* sample_mean_distribution(int sample_size, int n_samples);

/* L4: LLN — running mean convergence */
double* llN_running_mean(int n);

/* L5: Multivariate Gaussian */
double multivariate_gaussian_logpdf(double* x, double* mu, double* sigma_sq, int k);

/* L7: Rayleigh — signal processing */
double rayleigh_pdf(double x, double sigma);
double rayleigh_expectation(double sigma);
double rayleigh_variance(double sigma);

/* L7: Geometric — count failures */
double geometric_pmf(int k, double p);
double geometric_expectation(double p);
double geometric_variance(double p);

#endif
