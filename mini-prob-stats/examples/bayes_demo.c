#include "bayes.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int main(void) {
    srand((unsigned)time(NULL));

    printf("=== Beta-Binomial Bayesian Inference ===\n\n");

    BetaBinomial* bb = bayes_beta_binomial_create(2.0, 2.0);
    printf("Prior: Beta(2.0, 2.0)\n");
    printf("Prior mean: %.4f\n\n", bayes_posterior_mean(bb));

    int observed_successes = 7;
    int observed_trials = 10;
    bayes_update(bb, observed_successes, observed_trials);
    printf("Observed: %d successes out of %d trials\n", observed_successes, observed_trials);

    double post_mean = bayes_posterior_mean(bb);
    double post_var  = bayes_posterior_variance(bb);
    double post_sd   = sqrt(post_var);

    printf("\nPosterior: Beta(%.1f, %.1f)\n",
           bb->alpha + bb->successes, bb->beta + bb->trials - bb->successes);
    printf("Posterior mean:      %.4f\n", post_mean);
    printf("Posterior variance:  %.6f\n", post_var);
    printf("Posterior std dev:   %.4f\n", post_sd);
    printf("Predictive P(next=1): %.4f\n", bayes_predictive(bb));

    double lo, hi;
    bayes_credible_interval(bb, 0.95, &lo, &hi);
    printf("95%% Credible interval: [%.4f, %.4f]\n", lo, hi);

    bayes_free(bb);

    printf("\n=== Naive Bayes Classifier ===\n\n");

    double data[10] = {1.0, 2.0, 1.5, 2.5, 1.8,  5.0, 6.0, 5.5, 6.5, 5.2};
    int labels[10]   = {0,   0,   0,   0,   0,    1,   1,   1,   1,   1};

    NaiveBayes* nb = naive_bayes_create();
    naive_bayes_fit(nb, data, labels, 10);

    printf("Class 0 prior: %.4f, mean: %.4f, variance: %.4f\n",
           nb->prior[0], nb->mean[0], nb->variance[0]);
    printf("Class 1 prior: %.4f, mean: %.4f, variance: %.4f\n",
           nb->prior[1], nb->mean[1], nb->variance[1]);

    double test_vals[] = {1.0, 2.0, 3.5, 5.0, 6.0};
    for (int i = 0; i < 5; i++) {
        int pred = naive_bayes_predict(nb, test_vals[i]);
        printf("x=%.1f -> class %d\n", test_vals[i], pred);
    }

    naive_bayes_free(nb);
    return 0;
}
