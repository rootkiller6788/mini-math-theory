/* ab_test_demo.c ¡ª Bayesian A/B Testing Demo
 *
 * Demonstrates:
 *   - Beta-Binomial conjugate updating
 *   - Bayesian A/B test: P(B > A)
 *   - Expected lift calculation
 *   - Sequential updating
 *
 * Application: online experimentation, clinical trials, conversion optimization
 */

#include "bayes.h"
#include "distribution.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    srand((unsigned)time(NULL));
    printf("=== Bayesian A/B Testing Demo ===\n\n");

    /* Scenario: A = control (10% conversion), B = variant (15% conversion)
     * We observe: A: 10/100 (10%), B: 15/100 (15%) */
    int k_a = 10, n_a = 100;
    int k_b = 15, n_b = 100;

    printf("Conversion data:\n");
    printf("  Variant A (control): %d / %d = %.1f%%\n", k_a, n_a, 100.0 * k_a / n_a);
    printf("  Variant B (test):    %d / %d = %.1f%%\n\n", k_b, n_b, 100.0 * k_b / n_b);

    /* Bayesian analysis with Beta(1,1) prior */
    double prob_b_better = bayes_ab_test_prob_b_better(k_a, n_a, k_b, n_b, 5000);
    double expected_lift = bayes_ab_test_expected_lift(k_a, n_a, k_b, n_b);

    printf("Bayesian analysis (Beta(1,1) prior):\n");
    printf("  P(B > A) = %.4f (%.1f%%)\n", prob_b_better, 100.0 * prob_b_better);
    printf("  Expected lift: %.4f (%.1f%%)\n\n",
           expected_lift, 100.0 * expected_lift);

    if (prob_b_better > 0.95) {
        printf("  ==> Strong evidence: B is better than A!\n");
    } else if (prob_b_better > 0.8) {
        printf("  ==> Moderate evidence: B likely better than A.\n");
    } else {
        printf("  ==> Insufficient evidence. Collect more data.\n");
    }

    /* Sequential updating demo */
    printf("\n--- Sequential Beta-Binomial Update ---\n");
    BetaBinomial* bb = bayes_beta_binomial_create(1.0, 1.0);
    printf("Prior: Beta(1.0, 1.0), mean=%.4f\n", bayes_posterior_mean(bb));

    int outcomes[] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 1};  /* 6 heads, 4 tails */
    printf("Observing outcomes: ");
    for (int i = 0; i < 10; i++) printf("%d ", outcomes[i]);
    printf("\n");

    for (int i = 0; i < 10; i++) {
        bayes_update(bb, outcomes[i], 1);
        printf("  After %2d: mean=%.4f, Beta(%.1f, %.1f)\n",
               i+1, bayes_posterior_mean(bb),
               bb->alpha + bb->successes, bb->beta + bb->trials - bb->successes);
    }

    double lo, hi;
    bayes_credible_interval(bb, 0.95, &lo, &hi);
    printf("\n95%% Credible interval: [%.4f, %.4f]\n", lo, hi);

    bayes_free(bb);

    /* Bayes Factor demo */
    printf("\n--- Bayes Factor: fair coin vs biased coin ---\n");
    /* H0: p = 0.5 (Beta(10,10) prior around 0.5)
     * H1: p > 0.5 (Beta(2,1) prior favoring >0.5)
     * Data: 7 heads in 10 flips */
    double bf = bayes_factor_beta_binomial(10, 10, 2, 1, 7, 10);
    printf("  Data: 7 heads in 10 flips\n");
    printf("  BF(H1 vs H0) = %.4f\n", bf);
    if (bf > 3) {
        printf("  ==> Substantial evidence for biased coin.\n");
    } else {
        printf("  ==> Weak or no evidence for biased coin.\n");
    }

    return 0;
}
