#ifndef BAYES_H
#define BAYES_H

/* L2: Beta-Binomial conjugate model */
typedef struct {
    double alpha, beta;
    int successes, trials;
} BetaBinomial;

BetaBinomial* bayes_beta_binomial_create(double alpha, double beta);
void bayes_update(BetaBinomial* bb, int successes, int trials);
double bayes_posterior_mean(BetaBinomial* bb);
double bayes_posterior_variance(BetaBinomial* bb);
double bayes_predictive(BetaBinomial* bb);
void bayes_credible_interval(BetaBinomial* bb, double prob, double* lo, double* hi);
void bayes_free(BetaBinomial* bb);

/* L4: Bayes Factor for Beta-Binomial model comparison */
double bayes_factor_beta_binomial(double alpha0, double beta0,
                                   double alpha1, double beta1,
                                   int k, int n);

/* L2: Normal-Normal conjugate model */
typedef struct {
    double mu0, tau0, sigma;
    double posterior_mean, posterior_sd;
    int n_obs;
} NormalNormal;

NormalNormal* bayes_normal_normal_create(double mu0, double tau0, double sigma);
void bayes_normal_normal_update(NormalNormal* nn, double* data, int n);
void bayes_normal_normal_free(NormalNormal* nn);

/* L2: Gamma-Poisson conjugate model */
typedef struct {
    double alpha, beta;
    double sum_data;
    int n_data;
} GammaPoisson;

GammaPoisson* bayes_gamma_poisson_create(double alpha, double beta);
void bayes_gamma_poisson_update(GammaPoisson* gp, int* data, int n);
double bayes_gamma_poisson_posterior_mean(GammaPoisson* gp);
void bayes_gamma_poisson_free(GammaPoisson* gp);

/* L5: Naive Bayes classifier (Gaussian) */
typedef struct {
    double prior[2];
    double mean[2];
    double variance[2];
} NaiveBayes;

NaiveBayes* naive_bayes_create(void);
void naive_bayes_fit(NaiveBayes* nb, double* data, int* labels, int n);
int naive_bayes_predict(NaiveBayes* nb, double x);
void naive_bayes_free(NaiveBayes* nb);

/* L7: Bayesian A/B testing */
double bayes_ab_test_prob_b_better(int k_a, int n_a, int k_b, int n_b,
                                    int n_samples);
double bayes_ab_test_expected_lift(int k_a, int n_a, int k_b, int n_b);

/* L3: Sequential updating */
void bayes_sequential_update(BetaBinomial* bb, int* outcomes, int m);

#endif
