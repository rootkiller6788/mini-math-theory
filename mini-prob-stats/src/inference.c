/* inference.c — Statistical inference: estimation, confidence, model selection.
 *
 * L4: MLE (Maximum Likelihood Estimation), Method of Moments
 * L4: Confidence Intervals (Gaussian mean, proportion, difference of means)
 * L5: Bootstrap CI, EM algorithm, Kernel Density Estimation
 * L5: AIC/BIC model selection, Fisher Information, Likelihood Ratio Test
 *
 * Theorem sources:
 *   Casella & Berger §5–§10 (Point Estimation, CI, Hypothesis Testing)
 *   MIT 18.05 §8 (Bayesian vs Frequentist)
 *   CS229 §1 (Linear Algebra, Stats review)
 *   Efron & Tibshirani §12–§14 (Bootstrap)
 */

#include "inference.h"
#include "distribution.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

/* --------------------------------------------------------------------------
 * L1: Descriptive statistics — mean, variance, stddev, median, percentile
 * -------------------------------------------------------------------------- */
double mean(double* data, int n) {
    if (n <= 0) return 0.0;
    double s = 0.0;
    for (int i = 0; i < n; i++) s += data[i];
    return s / n;
}

double variance(double* data, int n) {
    if (n <= 1) return 0.0;
    double m = mean(data, n);
    double ss = 0.0;
    for (int i = 0; i < n; i++) {
        double d = data[i] - m;
        ss += d * d;
    }
    return ss / (n - 1);
}

double stddev(double* data, int n) {
    return sqrt(variance(data, n));
}

static int cmp_double(const void* a, const void* b) {
    double da = *(const double*)a;
    double db = *(const double*)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

double median(double* data, int n) {
    if (n <= 0) return 0.0;
    double* sorted = (double*)malloc(n * sizeof(double));
    if (!sorted) return 0.0;
    memcpy(sorted, data, n * sizeof(double));
    qsort(sorted, n, sizeof(double), cmp_double);
    double m;
    if (n % 2 == 0) {
        m = (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0;
    } else {
        m = sorted[n / 2];
    }
    free(sorted);
    return m;
}

double percentile(double* data, int n, double p) {
    if (n <= 0) return 0.0;
    if (p < 0.0) p = 0.0;
    if (p > 100.0) p = 100.0;
    double* sorted = (double*)malloc(n * sizeof(double));
    if (!sorted) return 0.0;
    memcpy(sorted, data, n * sizeof(double));
    qsort(sorted, n, sizeof(double), cmp_double);
    double rank = (p / 100.0) * (n - 1);
    int lo = (int)rank;
    double frac = rank - lo;
    double result;
    if (lo >= n - 1) {
        result = sorted[n - 1];
    } else {
        result = sorted[lo] + frac * (sorted[lo + 1] - sorted[lo]);
    }
    free(sorted);
    return result;
}

double covariance(double* x, double* y, int n) {
    if (n <= 1) return 0.0;
    double mx = mean(x, n);
    double my = mean(y, n);
    double ss = 0.0;
    for (int i = 0; i < n; i++) {
        ss += (x[i] - mx) * (y[i] - my);
    }
    return ss / (n - 1);
}

double correlation(double* x, double* y, int n) {
    double cov = covariance(x, y, n);
    double sx = stddev(x, n);
    double sy = stddev(y, n);
    if (sx == 0.0 || sy == 0.0) return 0.0;
    return cov / (sx * sy);
}

/* IQR: Q₃ - Q₁, robust measure of spread */
double iqr(double* data, int n) {
    return percentile(data, n, 75.0) - percentile(data, n, 25.0);
}

/* MAD: Median Absolute Deviation, robust scale estimator */
double mad(double* data, int n) {
    if (n <= 0) return 0.0;
    double med = median(data, n);
    double* abs_dev = (double*)malloc(n * sizeof(double));
    if (!abs_dev) return 0.0;
    for (int i = 0; i < n; i++) {
        abs_dev[i] = fabs(data[i] - med);
    }
    double result = 1.4826 * median(abs_dev, n);
    free(abs_dev);
    return result;
}

/* --------------------------------------------------------------------------
 * L4: Maximum Likelihood Estimation (MLE)
 *   Binomial:    p̂ = k/n
 *   Poisson:     λ̂ = X̄
 *   Gaussian:    μ̂ = X̄, σ̂² = (1/n)Σ(Xᵢ-X̄)²
 *   Exponential: λ̂ = 1/X̄
 *   Uniform:     â = min Xᵢ, b̂ = max Xᵢ (method of moments variant)
 * -------------------------------------------------------------------------- */
double mle_binomial_p(int k, int n) {
    if (n <= 0) return 0.0;
    return (double)k / n;
}

double mle_poisson_lambda(double* data, int n) {
    return mean(data, n);
}

double mle_gaussian_mu(double* data, int n) {
    return mean(data, n);
}

double mle_gaussian_sigma(double* data, int n) {
    if (n <= 0) return 0.0;
    double m = mean(data, n);
    double ss = 0.0;
    for (int i = 0; i < n; i++) {
        double d = data[i] - m;
        ss += d * d;
    }
    return sqrt(ss / n);  /* MLE uses n, not n-1 */
}

double mle_exponential_lambda(double* data, int n) {
    double m = mean(data, n);
    if (m <= 0.0) return 0.0;
    return 1.0 / m;
}

/* --------------------------------------------------------------------------
 * L4: Confidence Intervals (frequentist)
 *
 * CI for mean (known σ):
 *   X̄ ± z_{α/2} · σ/√n
 *
 * CI for proportion:
 *   p̂ ± z_{α/2} · √(p̂(1-p̂)/n)
 *
 * CI for difference of means:
 *   (X̄₁-X̄₂) ± z_{α/2} · √(σ₁²/n₁ + σ₂²/n₂)
 * -------------------------------------------------------------------------- */
static double z_for_confidence(double confidence) {
    double tail = (1.0 - confidence) / 2.0;
    return gaussian_quantile(1.0 - tail);
}

void ci_gaussian_mean(double* data, int n, double sigma, double confidence,
                      double* lo, double* hi) {
    if (n <= 0) { *lo = *hi = 0.0; return; }
    double xbar = mean(data, n);
    double z = z_for_confidence(confidence);
    double margin = z * sigma / sqrt((double)n);
    *lo = xbar - margin;
    *hi = xbar + margin;
}

void ci_proportion(int k, int n, double confidence, double* lo, double* hi) {
    if (n <= 0) { *lo = *hi = 0.0; return; }
    double p_hat = (double)k / n;
    double z = z_for_confidence(confidence);
    double margin = z * sqrt(p_hat * (1.0 - p_hat) / n);
    *lo = p_hat - margin;
    *hi = p_hat + margin;
    if (*lo < 0.0) *lo = 0.0;
    if (*hi > 1.0) *hi = 1.0;
}

void ci_diff_means(double* d1, int n1, double* d2, int n2, double confidence,
                   double* lo, double* hi) {
    if (n1 <= 0 || n2 <= 0) { *lo = *hi = 0.0; return; }
    double xbar1 = mean(d1, n1);
    double xbar2 = mean(d2, n2);
    double v1 = variance(d1, n1);
    double v2 = variance(d2, n2);
    double z = z_for_confidence(confidence);
    double se = sqrt(v1 / n1 + v2 / n2);
    double diff = xbar1 - xbar2;
    double margin = z * se;
    *lo = diff - margin;
    *hi = diff + margin;
}

/* CI for variance (chi-squared based):
 *   [(n-1)s²/χ²_{α/2,n-1}, (n-1)s²/χ²_{1-α/2,n-1}]
 * L4: Uses the fact that (n-1)S²/σ² ~ χ²_{n-1}
 * Approximation: normal quantiles for large n
 */
void ci_variance(double* data, int n, double confidence, double* lo, double* hi) {
    if (n <= 1) { *lo = *hi = 0.0; return; }
    double s2 = variance(data, n);
    double z = z_for_confidence(confidence);
    double se = s2 * sqrt(2.0 / (n - 1));
    *lo = s2 - z * se;
    *hi = s2 + z * se;
    if (*lo < 0.0) *lo = 0.0;
}

/* --------------------------------------------------------------------------
 * L5: Bootstrap Confidence Interval (percentile method)
 * Non-parametric, resampling-based.
 * Efron (1979): The bootstrap is a computer-based method for assigning
 * measures of accuracy to statistical estimates.
 *
 * Complexity: O(B · n) where B = n_bootstrap iterations
 * -------------------------------------------------------------------------- */
double* bootstrap_resample(double* data, int n) {
    double* resample = (double*)malloc(n * sizeof(double));
    if (!resample) return NULL;
    for (int i = 0; i < n; i++) {
        resample[i] = data[rand() % n];
    }
    return resample;
}

void bootstrap_ci(double* data, int n, int n_bootstrap, double confidence,
                  double* lo, double* hi) {
    if (n <= 0 || n_bootstrap <= 0 || !data) {
        *lo = *hi = 0.0; return;
    }
    double* boot_means = (double*)malloc(n_bootstrap * sizeof(double));
    if (!boot_means) { *lo = *hi = 0.0; return; }

    for (int b = 0; b < n_bootstrap; b++) {
        double* resamp = bootstrap_resample(data, n);
        boot_means[b] = mean(resamp, n);
        free(resamp);
    }

    qsort(boot_means, n_bootstrap, sizeof(double), cmp_double);

    double alpha = (1.0 - confidence) / 2.0;
    int idx_lo = (int)(alpha * n_bootstrap);
    int idx_hi = (int)((1.0 - alpha) * n_bootstrap);
    if (idx_lo < 0) idx_lo = 0;
    if (idx_hi >= n_bootstrap) idx_hi = n_bootstrap - 1;

    *lo = boot_means[idx_lo];
    *hi = boot_means[idx_hi];
    free(boot_means);
}

/* --------------------------------------------------------------------------
 * L5: EM Algorithm — Gaussian Mixture Model (2 components, 1D)
 *
 * Dempster, Laird, Rubin (1977).
 * Alternates E-step (compute responsibilities) and M-step (update params).
 *
 * Model: f(x) = π₁·N(μ₁,σ₁²) + π₂·N(μ₂,σ₂²), π₁+π₂=1
 * Complexity: O(k · n · max_iter) per run
 * -------------------------------------------------------------------------- */
void em_gaussian_mixture_1d(double* data, int n,
                            double* pi, double* mu, double* sigma,
                            int max_iter, double tol) {
    if (n < 2 || max_iter < 1) return;

    /* Initialize: k-means-like split at median */
    double med = median(data, n);
    double sums[2] = {0.0, 0.0};
    int cnts[2] = {0, 0};
    for (int i = 0; i < n; i++) {
        int c = (data[i] < med) ? 0 : 1;
        sums[c] += data[i];
        cnts[c]++;
    }
    for (int c = 0; c < 2; c++) {
        if (cnts[c] > 0) {
            mu[c] = sums[c] / cnts[c];
        } else {
            mu[c] = med + (c == 0 ? -1.0 : 1.0);
        }
        pi[c] = (double)cnts[c] / n;
    }
    for (int c = 0; c < 2; c++) {
        double ss = 0.0;
        int cc = 0;
        for (int i = 0; i < n; i++) {
            int cls = (data[i] < med) ? 0 : 1;
            if (cls == c) { double d = data[i] - mu[c]; ss += d * d; cc++; }
        }
        sigma[c] = (cc > 1) ? sqrt(ss / (cc)) : 1.0;
        if (sigma[c] < 0.01) sigma[c] = 0.01;
    }

    double* resp = (double*)malloc(n * 2 * sizeof(double));
    if (!resp) return;

    for (int iter = 0; iter < max_iter; iter++) {
        /* E-step: compute responsibilities γ_{ic} */
        for (int i = 0; i < n; i++) {
            double dens[2];
            double total = 0.0;
            for (int c = 0; c < 2; c++) {
                dens[c] = pi[c] * gaussian_pdf(data[i], mu[c], sigma[c]);
                total += dens[c];
            }
            if (total < 1e-300) total = 1e-300;
            for (int c = 0; c < 2; c++) {
                resp[i * 2 + c] = dens[c] / total;
            }
        }

        /* M-step: update π, μ, σ */
        double old_mu[2] = {mu[0], mu[1]};
        double Nc[2] = {0.0, 0.0};
        for (int c = 0; c < 2; c++) {
            for (int i = 0; i < n; i++) Nc[c] += resp[i * 2 + c];
        }
        for (int c = 0; c < 2; c++) {
            pi[c] = Nc[c] / n;
            if (Nc[c] > 0.0) {
                mu[c] = 0.0;
                for (int i = 0; i < n; i++) mu[c] += resp[i * 2 + c] * data[i];
                mu[c] /= Nc[c];
            }
        }
        for (int c = 0; c < 2; c++) {
            if (Nc[c] > 0.0) {
                sigma[c] = 0.0;
                for (int i = 0; i < n; i++) {
                    double d = data[i] - mu[c];
                    sigma[c] += resp[i * 2 + c] * d * d;
                }
                sigma[c] = sqrt(sigma[c] / Nc[c]);
                if (sigma[c] < 0.01) sigma[c] = 0.01;
            }
        }

        /* Convergence check */
        double max_change = 0.0;
        for (int c = 0; c < 2; c++) {
            double chg = fabs(mu[c] - old_mu[c]);
            if (chg > max_change) max_change = chg;
        }
        if (max_change < tol) break;
    }
    free(resp);
}

/* --------------------------------------------------------------------------
 * L5: Kernel Density Estimation (KDE)
 * f̂(x) = (1/(nh)) Σᵢ K((x-Xᵢ)/h)
 *
 * Gaussian kernel, Silverman's rule-of-thumb bandwidth:
 *   h = 1.06 · σ̂ · n^{-1/5}
 *
 * Reference: Silverman (1986), Wand & Jones (1995)
 * Complexity: O(n) per evaluation
 * -------------------------------------------------------------------------- */
static double gaussian_kernel(double z) {
    return exp(-0.5 * z * z) / sqrt(2.0 * PI);
}

double kde_bandwidth_silverman(double* data, int n) {
    if (n <= 1) return 1.0;
    double s = stddev(data, n);
    if (s <= 0.0) s = 1.0;
    return 1.06 * s * pow((double)n, -0.2);
}

double kde_evaluate(double x, double* data, int n, double bandwidth) {
    if (n <= 0 || bandwidth <= 0.0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += gaussian_kernel((x - data[i]) / bandwidth);
    }
    return sum / (n * bandwidth);
}

/* KDE on a grid for plotting/density estimation */
void kde_grid(double* data, int n, double bandwidth,
              double* grid_x, double* grid_density, int grid_n,
              double x_min, double x_max) {
    if (n <= 0 || grid_n <= 0) return;
    for (int i = 0; i < grid_n; i++) {
        grid_x[i] = x_min + (x_max - x_min) * i / (grid_n - 1);
        grid_density[i] = kde_evaluate(grid_x[i], data, n, bandwidth);
    }
}

/* --------------------------------------------------------------------------
 * L5: AIC and BIC — model selection criteria
 *
 * AIC = -2 log L(θ̂) + 2k            (Akaike, 1974)
 * BIC = -2 log L(θ̂) + k·log(n)      (Schwarz, 1978)
 *
 * For normal models:
 *   log L(θ̂) = -n/2 log(2πσ̂²) - (1/(2σ̂²)) Σ(yᵢ-ŷᵢ)²
 * -------------------------------------------------------------------------- */
double compute_aic(int n, double rss, int k_params) {
    if (n <= 0) return 0.0;
    double sigma2_hat = rss / n;
    if (sigma2_hat <= 0.0) sigma2_hat = 1e-10;
    double log_lik = -0.5 * n * log(2.0 * PI * sigma2_hat) - 0.5 * n;
    return -2.0 * log_lik + 2.0 * k_params;
}

double compute_bic(int n, double rss, int k_params) {
    if (n <= 0) return 0.0;
    double sigma2_hat = rss / n;
    if (sigma2_hat <= 0.0) sigma2_hat = 1e-10;
    double log_lik = -0.5 * n * log(2.0 * PI * sigma2_hat) - 0.5 * n;
    return -2.0 * log_lik + k_params * log((double)n);
}

/* --------------------------------------------------------------------------
 * L5: Fisher Information (observed) for common models
 *
 * Binomial:  I(p) = n/(p(1-p))
 * Poisson:   I(λ) = n/λ
 * Gaussian mean (known σ): I(μ) = n/σ²
 * -------------------------------------------------------------------------- */
double fisher_info_binomial(int n, double p) {
    if (p <= 0.0 || p >= 1.0) return 0.0;
    return n / (p * (1.0 - p));
}

double fisher_info_poisson(int n, double lambda) {
    if (lambda <= 0.0) return 0.0;
    return n / lambda;
}

double fisher_info_gaussian_mu(int n, double sigma) {
    if (sigma <= 0.0) return 0.0;
    return n / (sigma * sigma);
}

/* --------------------------------------------------------------------------
 * L5: Likelihood Ratio Test
 *
 * Λ = L(θ₀|x) / L(θ̂|x)
 * -2 log Λ ~ χ²₁ (under H₀, Wilks' theorem)
 *
 * For testing μ = μ₀ (known σ):
 *   -2 log Λ = n (X̄-μ₀)² / σ²
 * -------------------------------------------------------------------------- */
double likelihood_ratio_statistic(double* data, int n, double mu0, double sigma) {
    if (n <= 0 || sigma <= 0.0) return 0.0;
    double xbar = mean(data, n);
    return n * (xbar - mu0) * (xbar - mu0) / (sigma * sigma);
}

/* --------------------------------------------------------------------------
 * L6: Jackknife — leave-one-out estimation
 * Used for bias correction and variance estimation.
 * Complexity: O(n²)
 * -------------------------------------------------------------------------- */
double jackknife_estimate(double* data, int n, double (*estimator)(double*, int)) {
    if (n <= 1) return 0.0;
    double full = estimator(data, n);
    double sum_jk = 0.0;
    double* subset = (double*)malloc((n - 1) * sizeof(double));
    if (!subset) return full;

    for (int i = 0; i < n; i++) {
        int idx = 0;
        for (int j = 0; j < n; j++) {
            if (j != i) subset[idx++] = data[j];
        }
        sum_jk += estimator(subset, n - 1);
    }
    free(subset);

    double pseudo_avg = sum_jk / n;
    return n * full - (n - 1) * pseudo_avg;
}

double jackknife_variance(double* data, int n, double (*estimator)(double*, int)) {
    if (n <= 1) return 0.0;
    double full = estimator(data, n);
    double* pseudo = (double*)malloc(n * sizeof(double));
    double* subset = (double*)malloc((n - 1) * sizeof(double));
    if (!pseudo || !subset) { free(pseudo); free(subset); return 0.0; }

    for (int i = 0; i < n; i++) {
        int idx = 0;
        for (int j = 0; j < n; j++) {
            if (j != i) subset[idx++] = data[j];
        }
        pseudo[i] = n * full - (n - 1) * estimator(subset, n - 1);
    }

    double mean_pseudo = 0.0;
    for (int i = 0; i < n; i++) mean_pseudo += pseudo[i];
    mean_pseudo /= n;

    double var = 0.0;
    for (int i = 0; i < n; i++) {
        double d = pseudo[i] - mean_pseudo;
        var += d * d;
    }
    var /= n;  /* jackknife variance of pseudo-values */

    free(pseudo);
    free(subset);
    return var / n;
}
