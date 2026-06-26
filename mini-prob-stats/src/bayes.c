/* bayes.c — Bayesian inference: conjugate models, Naive Bayes, Bayes factors.
 *
 * L2: Conjugate priors (Beta-Binomial, Normal-Normal, Gamma-Poisson)
 * L5: Naive Bayes classifier
 * L4: Bayes factor (model comparison)
 * L7: Bayesian A/B testing, Beta-Geometric model
 *
 * Theorem sources:
 *   MIT 18.05 §7–§8 (Bayesian Inference)
 *   CS229 §5 (Generative Learning Algorithms)
 *   Kass & Raftery (1995): Bayes Factors
 */

#include "bayes.h"
#include "distribution.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

/* --------------------------------------------------------------------------
 * L2: Beta-Binomial conjugate model
 *
 * Prior: θ ~ Beta(α, β)
 * Likelihood: X|θ ~ Binomial(n, θ)
 * Posterior: θ|X ~ Beta(α + k, β + n - k)
 *
 * Predictive: P(next=1) = (α+k)/(α+β+n)
 * -------------------------------------------------------------------------- */
BetaBinomial* bayes_beta_binomial_create(double alpha, double beta) {
    BetaBinomial* bb = (BetaBinomial*)malloc(sizeof(BetaBinomial));
    if (!bb) return NULL;
    bb->alpha = alpha;
    bb->beta = beta;
    bb->successes = 0;
    bb->trials = 0;
    return bb;
}

void bayes_update(BetaBinomial* bb, int successes, int trials) {
    if (!bb) return;
    bb->successes += successes;
    bb->trials += trials;
}

double bayes_posterior_mean(BetaBinomial* bb) {
    if (!bb) return 0.0;
    double a = bb->alpha + bb->successes;
    double b = bb->beta + bb->trials - bb->successes;
    return a / (a + b);
}

double bayes_posterior_variance(BetaBinomial* bb) {
    if (!bb) return 0.0;
    double a = bb->alpha + bb->successes;
    double b = bb->beta + bb->trials - bb->successes;
    return (a * b) / ((a + b) * (a + b) * (a + b + 1.0));
}

double bayes_predictive(BetaBinomial* bb) {
    return bayes_posterior_mean(bb);
}

void bayes_credible_interval(BetaBinomial* bb, double prob, double* lo, double* hi) {
    if (!bb) return;
    double m = bayes_posterior_mean(bb);
    double s = sqrt(bayes_posterior_variance(bb));
    double tail = (1.0 - prob) / 2.0;
    double z = 0.0;

    if (tail >= 0.025) {
        z = 1.96;   /* ~95% */
        if (tail >= 0.05) z = 1.645;
    }
    if (tail < 0.025 && tail >= 0.005) z = 2.576;
    if (tail < 0.005) z = 3.0;

    *lo = m - z * s;
    *hi = m + z * s;
    if (*lo < 0.0) *lo = 0.0;
    if (*hi > 1.0) *hi = 1.0;
}

void bayes_free(BetaBinomial* bb) {
    free(bb);
}

/* L4: Bayes Factor — evidence ratio for model comparison
 * BF₁₀ = P(D|M₁) / P(D|M₀)
 *
 * For Beta-Binomial: BF = Beta(α₁+k, β₁+n-k) / Beta(α₀, β₀)
 *                     / [Beta(α₀+k, β₀+n-k) / Beta(α₁, β₁)]
 *
 * log(BF) > 0 → evidence favors M₁
 * Interpretation: 0-2 → barely, 2-6 → positive, >6 → strong (Kass & Raftery)
 */
double bayes_factor_beta_binomial(double alpha0, double beta0,
                                   double alpha1, double beta1,
                                   int k, int n) {
    /* marginal P(D|M) = ∫ P(D|θ) π(θ) dθ = C(n,k) B(α+k, β+n-k) / B(α,β) */
    /* Log Bayes factor: */
    double log_bf = log_gamma_stirling(alpha1 + k) + log_gamma_stirling(beta1 + n - k)
                   + log_gamma_stirling(alpha0 + beta0)
                   - log_gamma_stirling(alpha1 + beta1 + n)
                   - log_gamma_stirling(alpha0) - log_gamma_stirling(beta0)
                   - log_gamma_stirling(alpha0 + k) - log_gamma_stirling(beta0 + n - k)
                   - log_gamma_stirling(alpha1) - log_gamma_stirling(beta1)
                   + log_gamma_stirling(alpha0 + beta0 + n)
                   + log_gamma_stirling(alpha1 + beta1);
    return exp(log_bf);
}

/* --------------------------------------------------------------------------
 * L2: Normal-Normal conjugate model
 *
 * Prior: μ ~ N(μ₀, τ₀²)
 * Likelihood: X|μ ~ N(μ, σ²), σ known
 * Posterior: μ|X̄ ~ N(μ_n, τ_n²)
 *   μ_n = (μ₀/τ₀² + nX̄/σ²) / (1/τ₀² + n/σ²)
 *   1/τ_n² = 1/τ₀² + n/σ²
 * precision = 1/variance
 * -------------------------------------------------------------------------- */
NormalNormal* bayes_normal_normal_create(double mu0, double tau0, double sigma) {
    NormalNormal* nn = (NormalNormal*)malloc(sizeof(NormalNormal));
    if (!nn) return NULL;
    nn->mu0 = mu0;
    nn->tau0 = tau0;
    nn->sigma = sigma;
    nn->posterior_mean = mu0;
    nn->posterior_sd = tau0;
    nn->n_obs = 0;
    return nn;
}

void bayes_normal_normal_update(NormalNormal* nn, double* data, int n) {
    if (!nn || !data || n <= 0) return;
    nn->n_obs += n;
    double xbar = 0.0;
    for (int i = 0; i < n; i++) xbar += data[i];
    xbar /= n;

    double prior_prec = 1.0 / (nn->tau0 * nn->tau0);
    double total_prec = prior_prec + nn->n_obs / (nn->sigma * nn->sigma);
    nn->posterior_mean = (prior_prec * nn->mu0
                         + nn->n_obs * xbar / (nn->sigma * nn->sigma))
                         / total_prec;
    nn->posterior_sd = sqrt(1.0 / total_prec);
}

void bayes_normal_normal_free(NormalNormal* nn) {
    free(nn);
}

/* --------------------------------------------------------------------------
 * L2: Gamma-Poisson conjugate model
 *
 * Prior: λ ~ Gamma(α, β)
 * Likelihood: X|λ ~ Poisson(λ)
 * Posterior: λ|X ~ Gamma(α + Σxᵢ, β + n)
 * -------------------------------------------------------------------------- */
GammaPoisson* bayes_gamma_poisson_create(double alpha, double beta) {
    GammaPoisson* gp = (GammaPoisson*)malloc(sizeof(GammaPoisson));
    if (!gp) return NULL;
    gp->alpha = alpha;
    gp->beta = beta;
    gp->sum_data = 0.0;
    gp->n_data = 0;
    return gp;
}

void bayes_gamma_poisson_update(GammaPoisson* gp, int* data, int n) {
    if (!gp || !data) return;
    for (int i = 0; i < n; i++) {
        gp->sum_data += data[i];
    }
    gp->n_data += n;
}

double bayes_gamma_poisson_posterior_mean(GammaPoisson* gp) {
    if (!gp) return 0.0;
    return (gp->alpha + gp->sum_data) / (gp->beta + gp->n_data);
}

void bayes_gamma_poisson_free(GammaPoisson* gp) {
    free(gp);
}

/* --------------------------------------------------------------------------
 * L5: Naive Bayes (Gaussian) Classifier
 *
 * P(C_k|x) ∝ P(C_k) ∏ P(x_j|C_k)
 * Assumes conditional independence of features given class.
 * Optimal when features are truly independent (though often works anyway).
 * -------------------------------------------------------------------------- */
NaiveBayes* naive_bayes_create(void) {
    NaiveBayes* nb = (NaiveBayes*)malloc(sizeof(NaiveBayes));
    if (!nb) return NULL;
    nb->prior[0] = 0.5;
    nb->prior[1] = 0.5;
    nb->mean[0] = 0.0;
    nb->mean[1] = 0.0;
    nb->variance[0] = 1.0;
    nb->variance[1] = 1.0;
    return nb;
}

void naive_bayes_fit(NaiveBayes* nb, double* data, int* labels, int n) {
    if (!nb || !data || !labels) return;
    int cnt[2] = {0, 0};
    double sum[2] = {0.0, 0.0};

    for (int i = 0; i < n; i++) {
        int c = labels[i];
        cnt[c]++;
        sum[c] += data[i];
    }

    nb->prior[0] = (double)cnt[0] / n;
    nb->prior[1] = (double)cnt[1] / n;

    for (int c = 0; c < 2; c++) {
        if (cnt[c] > 0) {
            nb->mean[c] = sum[c] / cnt[c];
        }
    }

    double sumsq[2] = {0.0, 0.0};
    for (int i = 0; i < n; i++) {
        int c = labels[i];
        double d = data[i] - nb->mean[c];
        sumsq[c] += d * d;
    }

    for (int c = 0; c < 2; c++) {
        if (cnt[c] > 1) {
            nb->variance[c] = sumsq[c] / (cnt[c] - 1);
        }
        if (nb->variance[c] < 1e-10) nb->variance[c] = 1e-10;
    }
}

int naive_bayes_predict(NaiveBayes* nb, double x) {
    if (!nb) return 0;
    double best = -1e300;
    int best_class = 0;
    for (int c = 0; c < 2; c++) {
        double var = nb->variance[c];
        double d = x - nb->mean[c];
        double log_pdf = -0.5 * log(2.0 * PI * var) - (d * d) / (2.0 * var);
        double score = log(nb->prior[c]) + log_pdf;
        if (score > best) {
            best = score;
            best_class = c;
        }
    }
    return best_class;
}

void naive_bayes_free(NaiveBayes* nb) {
    free(nb);
}

/* --------------------------------------------------------------------------
 * L7: Bayesian A/B Test — posterior probability that B > A
 *
 * Given conversions: k_A out of n_A, k_B out of n_B
 * Prior: Beta(1,1) for each (uniform)
 * Posterior: P(p_B > p_A | data) = ∫₀¹ ∫_{p_A}¹ Beta pdfs
 * Using Monte Carlo approximation (importance sampling from Beta)
 * -------------------------------------------------------------------------- */
static double sample_beta(int a_int, int b_int) {
    /* Sample Beta(a,b) using Gamma method: G_a / (G_a + G_b)
     * where G_a ~ Gamma(a, 1) = sum of a exponential(1) random vars
     * For non-integer parts, add one more exponential scaled.
     */
    double ga = 0.0, gb = 0.0;
    for (int i = 0; i < a_int; i++) {
        double u = (double)rand() / RAND_MAX;
        if (u < 1e-10) u = 1e-10;
        ga -= log(u);
    }
    for (int i = 0; i < b_int; i++) {
        double u = (double)rand() / RAND_MAX;
        if (u < 1e-10) u = 1e-10;
        gb -= log(u);
    }
    return ga / (ga + gb);
}

double bayes_ab_test_prob_b_better(int k_a, int n_a, int k_b, int n_b,
                                    int n_samples) {
    if (n_samples <= 0 || n_a <= 0 || n_b <= 0) return 0.5;

    /* Posterior: p_A ~ Beta(1+k_A, 1+n_A-k_A), same for B */
    int a_a = 1 + k_a, b_a = 1 + n_a - k_a;
    int a_b = 1 + k_b, b_b = 1 + n_b - k_b;

    int count_b_better = 0;
    for (int s = 0; s < n_samples; s++) {
        double p_a = sample_beta(a_a, b_a);
        double p_b = sample_beta(a_b, b_b);
        if (p_b > p_a) count_b_better++;
    }
    return (double)count_b_better / n_samples;
}

/* L7: Bayesian A/B test expected loss (choose B if wrong):
 * E[loss|choose B] = P(A>B) * E[A-B | A>B]
 * Simplified: returns probability * expected lift */
double bayes_ab_test_expected_lift(int k_a, int n_a, int k_b, int n_b) {
    double alpha_a = 1.0 + k_a, beta_a = 1.0 + n_a - k_a;
    double alpha_b = 1.0 + k_b, beta_b = 1.0 + n_b - k_b;
    double mean_a = alpha_a / (alpha_a + beta_a);
    double mean_b = alpha_b / (alpha_b + beta_b);
    return mean_b - mean_a;
}

/* --------------------------------------------------------------------------
 * L3: Bayesian updating with multiple observations
 * Sequential update: posteriorₜ = prior given x₁, then posteriorₜ₊₁ using x₂
 * -----------------------------------------------------------r--------------
 * Shows that Bayesian updating is commutative (order-independent).
 * -------------------------------------------------------------------------- */
void bayes_sequential_update(BetaBinomial* bb, int* outcomes, int m) {
    if (!bb || !outcomes) return;
    for (int i = 0; i < m; i++) {
        if (outcomes[i] == 1) {
            bayes_update(bb, 1, 1);
        } else {
            bayes_update(bb, 0, 1);
        }
    }
}
