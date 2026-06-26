/* rate_distortion.c — Rate-Distortion Theory: R(D), Lloyd-Max, Quantization
 * Reference: Cover & Thomas, Ch. 10
 * MIT 6.441: Advanced topics (beyond core syllabus)
 *
 * Key formulas:
 *   R(D) = min I(X;X_hat) subject to E[d] <= D              [Def 10.2]
 *   Binary-Hamming: R(D) = H(p) - H(D)                      [Thm 10.3.1]
 *   Gaussian-SE:    R(D) = 0.5 log2(variance/D)             [Thm 10.3.2]
 *   Shannon lower bound: R(D) >= h(p) - 0.5 log2(2 pi e D) [Eq 10.107]
 */
#include "rate_distortion.h"
#include "entropy.h"
#include "kl_divergence.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E  2.71828182845904523536
#endif

/* ── L8: Blahut-Arimoto for Rate-Distortion ──────────────────────── */
RDResult* rd_blahut_arimoto(const double** distortion_matrix,
                             const double* source_dist,
                             int n_x, int n_xhat,
                             double slope, int max_iter) {
    /* Minimize F = I(X;X_hat) + s*E[d(X,X_hat)] over p(x_hat|x).
     * The optimal test channel: p(x_hat|x) ∝ q(x_hat)*exp(s*d(x,x_hat))
     * Algorithm (Cover & Thomas, Sec 10.8):
     * 1. Init p(x_hat|x) uniform
     * 2. q(x_hat) = Sum_x p(x)*p(x_hat|x)
     * 3. p(x_hat|x) = q(x_hat)*exp(s*d) / Sum q(x_hat)*exp(s*d)
     * 4. Repeat until convergence */
    if (!distortion_matrix || !source_dist || n_x <= 0 || n_xhat <= 0 || max_iter <= 0)
        return NULL;

    RDResult* result = malloc(sizeof(RDResult));
    if (!result) return NULL;
    result->rate = 0.0;
    result->distortion = 0.0;
    result->input_distribution = malloc(n_x * sizeof(double));
    result->test_channel = malloc(n_x * sizeof(double*));
    if (!result->input_distribution || !result->test_channel) {
        rd_result_free(result); return NULL;
    }
    for (int x = 0; x < n_x; x++) {
        result->test_channel[x] = calloc(n_xhat, sizeof(double));
        if (!result->test_channel[x]) { rd_result_free(result); return NULL; }
    }
    result->iterations = 0;

    /* Copy source distribution */
    memcpy(result->input_distribution, source_dist, n_x * sizeof(double));

    /* Initialize p(x_hat|x) uniformly */
    for (int x = 0; x < n_x; x++)
        for (int xh = 0; xh < n_xhat; xh++)
            result->test_channel[x][xh] = 1.0 / n_xhat;

    double* q = malloc(n_xhat * sizeof(double));
    double* new_channel_row = malloc(n_xhat * sizeof(double));
    if (!q || !new_channel_row) { free(q); free(new_channel_row); rd_result_free(result); return NULL; }

    for (int iter = 0; iter < max_iter; iter++) {
        /* Step 2: q(x_hat) = Sum_x p(x) p(x_hat|x) */
        for (int xh = 0; xh < n_xhat; xh++) {
            q[xh] = 0.0;
            for (int x = 0; x < n_x; x++)
                q[xh] += source_dist[x] * result->test_channel[x][xh];
        }

        /* Step 3: update p(x_hat|x) */
        double max_diff = 0.0;
        for (int x = 0; x < n_x; x++) {
            double sum = 0.0;
            for (int xh = 0; xh < n_xhat; xh++) {
                double w = q[xh] * exp(slope * distortion_matrix[x][xh]);
                new_channel_row[xh] = w;
                sum += w;
            }
            if (sum <= 0.0) sum = 1e-10;

            for (int xh = 0; xh < n_xhat; xh++) {
                double new_val = new_channel_row[xh] / sum;
                double diff = fabs(new_val - result->test_channel[x][xh]);
                if (diff > max_diff) max_diff = diff;
                result->test_channel[x][xh] = new_val;
            }
        }
        result->iterations = iter + 1;
        if (max_diff < 1e-8) break;
    }

    /* Compute R and D */
    double mi = 0.0, avg_dist = 0.0;
    for (int x = 0; x < n_x; x++) {
        for (int xh = 0; xh < n_xhat; xh++) {
            double p_x = source_dist[x];
            double p_xh_given_x = result->test_channel[x][xh];
            double p_joint = p_x * p_xh_given_x;
            double p_xh = 0.0;
            for (int xx = 0; xx < n_x; xx++)
                p_xh += source_dist[xx] * result->test_channel[xx][xh];

            if (p_joint > 0.0 && p_xh > 0.0)
                mi += p_joint * log2(p_xh_given_x / p_xh);
            avg_dist += p_joint * distortion_matrix[x][xh];
        }
    }

    result->rate = mi;
    result->distortion = avg_dist;

    free(q);
    free(new_channel_row);
    return result;
}

void rd_result_free(RDResult* result) {
    if (!result) return;
    if (result->test_channel) {
        free(result->test_channel[0]);
        free(result->test_channel);
    }
    free(result->input_distribution);
    free(result);
}

/* ── L8: Analytic R(D) Functions ────────────────────────────────── */
double rd_binary_hamming(double p, double D) {
    /* Ber(p) source, Hamming distortion.
     * R(D) = H(p) - H(D) for 0 <= D <= min(p, 1-p) */
    if (D <= 0.0) return entropy_binary(p);
    double p_max = p < (1.0 - p) ? p : (1.0 - p);
    if (D >= p_max) return 0.0;
    return entropy_binary(p) - entropy_binary(D);
}

double rd_gaussian_squared_error(double variance, double D) {
    /* N(0,sigma^2), squared error distortion.
     * R(D) = 0.5 log2(sigma^2/D) for 0 < D <= sigma^2 */
    if (D <= 0.0) return 1e10;
    if (D >= variance) return 0.0;
    return 0.5 * log2(variance / D);
}

double rd_shannon_lower_bound_gaussian(double variance, double D) {
    /* Shannon lower bound: R(D) >= h(p) - 0.5 log2(2 pi e D)
     * For Gaussian with squared error, this is tight. */
    if (D <= 0.0) return 1e10;
    double h_p = 0.5 * log2(2.0 * M_PI * M_E * variance);
    double h_q_max = 0.5 * log2(2.0 * M_PI * M_E * D);
    return h_p - h_q_max;
}

double rd_laplacian_absolute_error(double b, double D) {
    /* Laplacian source p(x) = (1/2b) exp(-|x|/b), absolute error.
     * Shannon lower bound: R(D) >= h_Laplace - log2(2e D) */
    if (D <= 0.0) return 1e10;
    if (D >= b) return 0.0;
    double h_laplace = log2(2.0 * M_E * b);
    double bound = h_laplace - log2(2.0 * M_E * D);
    return bound > 0.0 ? bound : 0.0;
}

/* ── L5: Lloyd-Max Quantizer ────────────────────────────────────── */
static double gaussian_pdf(double x, const void* params) {
    (void)params;
    return exp(-0.5 * x * x) / sqrt(2.0 * M_PI);
}

double lloyd_max_quantizer(double (*pdf)(double x, const void* params),
                            const void* params,
                            double a, double b, int n_levels,
                            double* levels, double* thresholds, int max_iter) {
    if (!pdf || n_levels < 2 || a >= b || !levels || !thresholds || max_iter <= 0)
        return 0.0;

    /* Initialize thresholds uniformly */
    double dx = (b - a) / n_levels;
    for (int i = 0; i <= n_levels; i++)
        thresholds[i] = a + i * dx;

    double prev_mse = 1e10;
    int n_intervals = 1000;

    for (int iter = 0; iter < max_iter; iter++) {
        /* Step 1: Compute reconstruction levels (centroids) */
        for (int i = 0; i < n_levels; i++) {
            double num = 0.0, den = 0.0;
            double ta = thresholds[i], tb = thresholds[i + 1];
            double h_step = (tb - ta) / n_intervals;
            for (int j = 0; j <= n_intervals; j++) {
                double x = ta + j * h_step;
                if (x < ta) x = ta;
                if (x > tb) x = tb;
                double fx = pdf(x, params);
                double w = (j == 0 || j == n_intervals) ? 0.5 : 1.0;
                num += w * x * fx;
                den += w * fx;
            }
            levels[i] = den > 0.0 ? (num * h_step / den) : (ta + tb) / 2.0;
        }

        /* Step 2: Decision thresholds at midpoints */
        for (int i = 1; i < n_levels; i++)
            thresholds[i] = (levels[i - 1] + levels[i]) / 2.0;
        thresholds[0] = a;
        thresholds[n_levels] = b;

        /* Step 3: Compute MSE */
        double mse = 0.0;
        for (int i = 0; i < n_levels; i++) {
            double isum = 0.0;
            double ta = thresholds[i], tb = thresholds[i + 1];
            double h_step = (tb - ta) / n_intervals;
            for (int j = 0; j <= n_intervals; j++) {
                double x = ta + j * h_step;
                if (x < ta) x = ta;
                if (x > tb) x = tb;
                double fx = pdf(x, params);
                double err = (x - levels[i]) * (x - levels[i]);
                double w = (j == 0 || j == n_intervals) ? 0.5 : 1.0;
                isum += w * err * fx;
            }
            mse += isum * h_step;
        }

        if (fabs(prev_mse - mse) < 1e-10) break;
        prev_mse = mse;
    }

    return prev_mse;
}

double uniform_quantizer_mse(int n_bits) {
    /* Uniform quantizer on [0,1] with 2^R levels.
     * MSE = Delta^2/12 = 1/(12 * 4^R) */
    return 1.0 / (12.0 * pow(4.0, (double)n_bits));
}

double gaussian_quantizer_mse(int n_bits) {
    /* At high rates, optimal Gaussian quantizer MSE approx:
     * MSE ≈ (pi*sqrt(3)/2) * 2^(-2R) * sigma^2
     * SNR in dB ≈ 6.02R - 4.35 */
    double sigma2 = 1.0;
    double factor = M_PI * sqrt(3.0) / 2.0;
    return factor * pow(2.0, -2.0 * n_bits) * sigma2;
}

double entropy_constrained_quantizer_rate(const double* bin_probs, int n_bins) {
    if (!bin_probs || n_bins <= 0) return 0.0;
    double h = 0.0;
    for (int i = 0; i < n_bins; i++)
        if (bin_probs[i] > 0.0)
            h -= bin_probs[i] * log2(bin_probs[i]);
    return h;
}

/* ── L6: Quantizer Benchmark ────────────────────────────────────── */
QuantizerBenchmark quantizer_benchmark(double snr_db, int n_bits,
                                         double source_variance) {
    QuantizerBenchmark bm = {0};
    bm.snr_db = snr_db;
    bm.rate = (double)n_bits;
    bm.distortion = gaussian_quantizer_mse(n_bits) * source_variance;
    bm.rd_bound_rate = rd_gaussian_squared_error(source_variance, bm.distortion);
    bm.gap_to_bound = (double)n_bits - bm.rd_bound_rate;
    return bm;
}
