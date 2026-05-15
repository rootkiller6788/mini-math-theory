#ifndef BAYES_H
#define BAYES_H

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

typedef struct {
    double prior[2];
    double mean[2];
    double variance[2];
} NaiveBayes;

NaiveBayes* naive_bayes_create(void);
void naive_bayes_fit(NaiveBayes* nb, double* data, int* labels, int n);
int naive_bayes_predict(NaiveBayes* nb, double x);
void naive_bayes_free(NaiveBayes* nb);

#endif
