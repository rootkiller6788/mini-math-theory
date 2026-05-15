#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#define PI 3.14159265358979323846

double bernoulli_pmf(int k, double p);
double bernoulli_expectation(double p);
double bernoulli_variance(double p);

double binomial_pmf(int k, int n, double p);
double binomial_expectation(int n, double p);
double binomial_variance(int n, double p);

double poisson_pmf(int k, double lambda);
double poisson_expectation(double lambda);
double poisson_variance(double lambda);

double gaussian_pdf(double x, double mu, double sigma);
double gaussian_cdf(double x, double mu, double sigma);
double gaussian_expectation(double mu, double sigma);
double gaussian_variance(double mu, double sigma);
double gaussian_quantile(double p);

double uniform_pdf(double x, double a, double b);
double uniform_expectation(double a, double b);
double uniform_variance(double a, double b);

double exponential_pdf(double x, double lambda);
double exponential_cdf(double x, double lambda);
double exponential_expectation(double lambda);
double exponential_variance(double lambda);

double* sample_mean_distribution(int sample_size, int n_samples);

#endif
