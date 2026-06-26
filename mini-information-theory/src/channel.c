/* channel.c — Channel Models: BSC, BEC, AWGN, Fading, Water-Filling
 * Reference: Cover & Thomas, Ch. 7-9
 * MIT 6.441: Weeks 8-10
 *
 * Key formulas:
 *   BSC capacity:    C = 1 - H(p)              [Theorem 7.1.1]
 *   BEC capacity:    C = 1 - α                 [Theorem 7.1.5]
 *   AWGN capacity:   C = ½ log₂(1 + P/N)       [Theorem 9.1.1]
 *   Water-filling:   P_i = max(0, μ - N_i)     [Theorem 9.4.1]
 *   Channel coding:  R < C(P) achievable       [Theorem 7.7.1]
 */
#include "channel.h"
#include "entropy.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif

/* ── L1: Binary Symmetric Channel ───────────────────────────────── */
void bsc_init(BinarySymmetricChannel* ch, double error_prob) {
    if (!ch) return;
    ch->error_prob = error_prob;
    ch->input = NULL;
    ch->output = NULL;
    ch->length = 0;
    srand((unsigned)time(NULL));
}

double bsc_capacity(const BinarySymmetricChannel* ch) {
    if (!ch) return 0.0;
    return 1.0 - entropy_binary(ch->error_prob);
}

void bsc_transmit(BinarySymmetricChannel* ch, const bool* input, int len, bool* output) {
    if (!ch || !input || !output || len <= 0) return;
    for (int i = 0; i < len; i++) {
        double r = (double)rand() / RAND_MAX;
        output[i] = (r < ch->error_prob) ? !input[i] : input[i];
    }
}

double bsc_error_rate(const BinarySymmetricChannel* ch, const bool* original, int len) {
    if (!ch || !ch->output || !original || len <= 0) return 0.0;
    int errors = 0;
    for (int i = 0; i < len; i++)
        if (ch->output[i] != original[i]) errors++;
    return (double)errors / len;
}

double bsc_mutual_information(double error_prob, double input_dist) {
    double p = input_dist;
    double pe = error_prob;
    double h_y = entropy_binary(p * (1 - pe) + (1 - p) * pe);
    double h_yx = entropy_binary(pe);
    return h_y - h_yx;
}

/* ── L2: Binary Erasure Channel ─────────────────────────────────── */
double bec_capacity(double erasure_prob) {
    /* C = 1 - α for BEC(α). Erasure breaks no information. */
    if (erasure_prob < 0.0) erasure_prob = 0.0;
    if (erasure_prob > 1.0) erasure_prob = 1.0;
    return 1.0 - erasure_prob;
}

void bec_transmit(const bool* input, int len, double erasure_prob,
                  int* output, bool* erased) {
    if (!input || !output || !erased || len <= 0) return;
    for (int i = 0; i < len; i++) {
        double r = (double)rand() / RAND_MAX;
        if (r < erasure_prob) {
            erased[i] = true;
            output[i] = 0;
        } else {
            erased[i] = false;
            output[i] = input[i] ? 1 : 0;
        }
    }
}

double bec_mutual_info(double erasure_prob, double input_dist) {
    /* I(X;Y) = (1 - α) * H(X) for BEC */
    double p = input_dist;
    return (1.0 - erasure_prob) * entropy_binary(p);
}

/* ── L2: Z-Channel ──────────────────────────────────────────────── */
double zchannel_capacity(double p_1to0) {
    /* Z-Channel: P(Y=1|X=1) = 1-p, P(Y=1|X=0) = 0, P(Y=0|X=1) = p, P(Y=0|X=0) = 1
     * Capacity: C = log₂(1 + (1-p) · p^{p/(1-p)})   [Cover & Thomas, Problem 7.8]
     * Computed via Blahut-Arimoto or analytically. */
    if (p_1to0 <= 0.0) return 1.0;
    if (p_1to0 >= 1.0) return 0.0;

    /* Use Blahut-Arimoto for 2-input, 2-output channel */
    GeneralChannel gc;
    channel_init(&gc, 2, 2);
    channel_set_transition(&gc, 0, 0, 1.0);
    channel_set_transition(&gc, 0, 1, 0.0);
    channel_set_transition(&gc, 1, 0, p_1to0);
    channel_set_transition(&gc, 1, 1, 1.0 - p_1to0);

    double cap = channel_capacity_blahut(&gc, 1e-6, 100);
    channel_free(&gc);
    return cap;
}

double zchannel_mutual_info(double p_1to0, double input_dist) {
    /* I(X;Y) for Z-channel with given input distribution */
    double p = input_dist;
    if (p <= 0.0 || p >= 1.0) return 0.0;

    double py1 = p * (1.0 - p_1to0);
    double h_y = entropy_binary(py1);
    double h_yx = p * entropy_binary(p_1to0); /* H(Y|X=1) = H_2(p), H(Y|X=0)=0 */

    return h_y - h_yx;
}

/* ── L3: AWGN Channel ───────────────────────────────────────────── */
double snr_db(double signal_power, double noise_power) {
    if (noise_power <= 0.0) return 1e10;
    return 10.0 * log10(signal_power / noise_power);
}

double awgn_capacity(double signal_power, double noise_variance) {
    /* C = ½ log₂(1 + P/N) bits per channel use (real channel)
     * Cover & Thomas, Theorem 9.1.1 */
    if (noise_variance <= 0.0) return 1e10;
    double snr = signal_power / noise_variance;
    return 0.5 * log2(1.0 + snr);
}

double awgn_capacity_bandwidth(double bandwidth, double signal_power, double noise_power) {
    /* C = B log₂(1 + P/(N₀B)) bits/second — Shannon-Hartley theorem */
    if (bandwidth <= 0.0 || noise_power <= 0.0) return 0.0;
    return bandwidth * log2(1.0 + signal_power / noise_power);
}

double awgn_capacity_spectral_efficiency(double snr_linear) {
    /* C/B = log₂(1 + SNR) bits/s/Hz */
    if (snr_linear <= 0.0) return 0.0;
    return log2(1.0 + snr_linear);
}

void awgn_transmit(const double* x, int n, double noise_std, double* y) {
    if (!x || !y || n <= 0) return;
    /* Box-Muller for Gaussian noise */
    for (int i = 0; i < n; i++) {
        double u1 = (double)rand() / RAND_MAX;
        double u2 = (double)rand() / RAND_MAX;
        if (u1 < 1e-10) u1 = 1e-10;
        double n_val = noise_std * sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
        y[i] = x[i] + n_val;
    }
}

/* ── L5: Fading Channel ─────────────────────────────────────────── */
double fading_ergodic_capacity(double avg_snr, int n_samples) {
    /* E[log₂(1 + |h|² SNR)] where |h|² ~ Exp(1) for Rayleigh fading.
     * Monte Carlo estimate. */
    if (avg_snr <= 0.0 || n_samples <= 0) return 0.0;

    double sum = 0.0;
    for (int i = 0; i < n_samples; i++) {
        /* Generate exponential(1) via inverse CDF */
        double u = (double)rand() / RAND_MAX;
        if (u < 1e-10) u = 1e-10;
        double h2 = -log(u); /* |h|² ~ Exp(1) */
        sum += log2(1.0 + h2 * avg_snr);
    }
    return sum / n_samples;
}

double fading_outage_probability(double target_rate, double avg_snr, int n_samples) {
    /* P(log₂(1 + |h|² SNR) < R) = P(|h|² < (2^R - 1)/SNR)
     * For Rayleigh: |h|² ~ Exp(1), so P(|h|² < x) = 1 - e^{-x}. */
    if (avg_snr <= 0.0 || n_samples <= 0) return 1.0;

    double threshold = (pow(2.0, target_rate) - 1.0) / avg_snr;
    if (threshold < 0.0) return 0.0;

    /* Analytic: 1 - exp(-threshold) */
    double analytic = 1.0 - exp(-threshold);

    /* Monte Carlo verification */
    int outages = 0;
    for (int i = 0; i < n_samples; i++) {
        double u = (double)rand() / RAND_MAX;
        if (u < 1e-10) u = 1e-10;
        double h2 = -log(u);
        if (log2(1.0 + h2 * avg_snr) < target_rate) outages++;
    }

    /* Return analytic value (more reliable than MC for small n_samples) */
    (void)outages;
    return analytic;
}

/* ── L8: Water-Filling ──────────────────────────────────────────── */
double water_filling_capacity(const double* noise_powers, int n_subchannels,
                               double total_power, double* power_allocation) {
    /* Solve P_i = max(0, μ - N_i) subject to Σ P_i = P_total.
     * Sort noise powers, find water level μ by binary search. */
    if (!noise_powers || n_subchannels <= 0 || total_power <= 0.0) return 0.0;

    /* Sort noise powers ascending */
    double* sorted_n = malloc(n_subchannels * sizeof(double));
    if (!sorted_n) return 0.0;
    memcpy(sorted_n, noise_powers, n_subchannels * sizeof(double));

    /* Simple bubble sort (n is small in practice) */
    for (int i = 0; i < n_subchannels; i++)
        for (int j = i + 1; j < n_subchannels; j++)
            if (sorted_n[i] > sorted_n[j]) {
                double tmp = sorted_n[i];
                sorted_n[i] = sorted_n[j];
                sorted_n[j] = tmp;
            }

    /* Binary search for water level μ */
    double low = sorted_n[0];
    double high = sorted_n[n_subchannels - 1] + total_power;
    double mu = (low + high) / 2.0;
    double capacity = 0.0;

    for (int iter = 0; iter < 100; iter++) {
        mu = (low + high) / 2.0;
        double sum_p = 0.0;
        for (int i = 0; i < n_subchannels; i++) {
            double pi = mu - sorted_n[i];
            if (pi > 0.0) sum_p += pi;
        }

        if (fabs(sum_p - total_power) < 1e-10)
            break;
        else if (sum_p > total_power)
            high = mu;
        else
            low = mu;
    }

    /* Compute final allocation and capacity */
    capacity = 0.0;
    for (int i = 0; i < n_subchannels; i++) {
        double pi = mu - sorted_n[i];
        if (pi < 0.0) pi = 0.0;
        if (power_allocation) power_allocation[i] = pi;
        capacity += 0.5 * log2(1.0 + pi / sorted_n[i]);
    }

    free(sorted_n);
    return capacity;
}

/* ── L1: General Channel ────────────────────────────────────────── */
void channel_init(GeneralChannel* ch, int in_size, int out_size) {
    if (!ch) return;
    ch->input_size = in_size;
    ch->output_size = out_size;
    ch->matrix = malloc(in_size * sizeof(double*));
    if (!ch->matrix) { ch->input_size = 0; return; }
    for (int i = 0; i < in_size; i++) {
        ch->matrix[i] = calloc(out_size, sizeof(double));
    }
}

void channel_set_transition(GeneralChannel* ch, int input, int output, double prob) {
    if (!ch || !ch->matrix) return;
    if (input >= 0 && input < ch->input_size &&
        output >= 0 && output < ch->output_size)
        ch->matrix[input][output] = prob;
}

void channel_free(GeneralChannel* ch) {
    if (!ch) return;
    for (int i = 0; i < ch->input_size; i++) free(ch->matrix[i]);
    free(ch->matrix);
    ch->matrix = NULL;
    ch->input_size = 0;
    ch->output_size = 0;
}

double channel_capacity_blahut(GeneralChannel* ch, double epsilon, int max_iter) {
    /* Blahut-Arimoto algorithm, IEEE Trans. IT, 1972.
     * Iteratively computes channel capacity by alternating optimization. */
    if (!ch || ch->input_size <= 0 || ch->output_size <= 0) return 0.0;

    int nx = ch->input_size, ny = ch->output_size;
    double* px = malloc(nx * sizeof(double));
    if (!px) return 0.0;
    for (int i = 0; i < nx; i++) px[i] = 1.0 / nx;

    for (int iter = 0; iter < max_iter; iter++) {
        double* py = calloc(ny, sizeof(double));
        if (!py) { free(px); return 0.0; }
        for (int j = 0; j < ny; j++)
            for (int i = 0; i < nx; i++)
                py[j] += px[i] * ch->matrix[i][j];

        double* new_px = calloc(nx, sizeof(double));
        if (!new_px) { free(py); free(px); return 0.0; }
        double norm = 0.0;

        for (int i = 0; i < nx; i++) {
            double sum = 0.0;
            for (int j = 0; j < ny; j++)
                if (ch->matrix[i][j] > 0 && py[j] > 0)
                    sum += ch->matrix[i][j] * log2(ch->matrix[i][j] / py[j]);
            new_px[i] = px[i] * pow(2.0, sum);
            norm += new_px[i];
        }

        double diff = 0.0;
        for (int i = 0; i < nx; i++) {
            if (norm > 0.0) new_px[i] /= norm;
            diff += fabs(new_px[i] - px[i]);
            px[i] = new_px[i];
        }

        free(py); free(new_px);
        if (diff < epsilon) break;
    }

    /* Compute final capacity */
    double* py = calloc(ny, sizeof(double));
    if (!py) { free(px); return 0.0; }
    for (int j = 0; j < ny; j++)
        for (int i = 0; i < nx; i++)
            py[j] += px[i] * ch->matrix[i][j];

    double cap = 0.0;
    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
            if (ch->matrix[i][j] > 0 && py[j] > 0)
                cap += px[i] * ch->matrix[i][j] * log2(ch->matrix[i][j] / py[j]);

    free(px); free(py);
    return cap;
}

double channel_capacity_uniform_input(GeneralChannel* ch) {
    if (!ch || ch->input_size <= 0) return 0.0;
    int nx = ch->input_size, ny = ch->output_size;
    double* px = malloc(nx * sizeof(double));
    if (!px) return 0.0;
    for (int i = 0; i < nx; i++) px[i] = 1.0 / nx;

    double* py = calloc(ny, sizeof(double));
    if (!py) { free(px); return 0.0; }
    for (int j = 0; j < ny; j++)
        for (int i = 0; i < nx; i++)
            py[j] += px[i] * ch->matrix[i][j];

    double mi = 0.0;
    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
            if (ch->matrix[i][j] > 0 && py[j] > 0)
                mi += px[i] * ch->matrix[i][j] * log2(ch->matrix[i][j] / py[j]);

    free(px); free(py);
    return mi;
}

/* ── L2: Symmetric Channel ──────────────────────────────────────── */
bool channel_is_symmetric(GeneralChannel* ch) {
    /* A channel is symmetric if every row is a permutation of every other row,
     * and every column is a permutation of every other column. */
    if (!ch || ch->input_size <= 0 || ch->output_size <= 0) return false;

    /* Check that each row has the same set of values */
    for (int i = 1; i < ch->input_size; i++) {
        bool matched[256] = {false};
        for (int j = 0; j < ch->output_size; j++) {
            bool found = false;
            for (int k = 0; k < ch->output_size; k++) {
                if (!matched[k] && fabs(ch->matrix[i][j] - ch->matrix[0][k]) < 1e-10) {
                    matched[k] = true;
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
    }

    /* Check that each column has the same sum (simplified uniform-input symmetry check) */
    double col_sum0 = 0.0;
    for (int i = 0; i < ch->input_size; i++) col_sum0 += ch->matrix[i][0];
    for (int j = 1; j < ch->output_size; j++) {
        double col_sum = 0.0;
        for (int i = 0; i < ch->input_size; i++) col_sum += ch->matrix[i][j];
        if (fabs(col_sum - col_sum0) > 1e-10) return false;
    }

    return true;
}

double channel_capacity_symmetric(GeneralChannel* ch) {
    /* For symmetric channels, capacity = log₂(|Y|) - H(row).
     * Achieved by uniform input. */
    if (!ch || !channel_is_symmetric(ch)) return channel_capacity_blahut(ch, 1e-6, 100);

    /* H(row) = -Σ P(y|x₀) log P(y|x₀) for any row */
    double h_row = 0.0;
    for (int j = 0; j < ch->output_size; j++)
        if (ch->matrix[0][j] > 0.0)
            h_row -= ch->matrix[0][j] * log2(ch->matrix[0][j]);

    return log2((double)ch->output_size) - h_row;
}

/* ── L4: Channel Coding Theorem Verification ────────────────────── */
double channel_coding_theorem_rate_bound(double error_prob, double target_error,
                                          int block_len) {
    /* For BSC(p): any rate R < C = 1 - H(p) is achievable with P_e → 0 as n → ∞.
     * Approximate bound: R < C - √(V/n) Q^{-1}(ε) + O(log n / n)
     * where V = p(1-p) log²(p/(1-p)) is the channel dispersion.
     * Returns the maximum rate for target error at given block length. */
    if (block_len <= 0 || error_prob <= 0 || error_prob >= 1) return 0.0;

    double cap = 1.0 - entropy_binary(error_prob);
    double dispersion = error_prob * (1.0 - error_prob) *
                        log2(error_prob / (1.0 - error_prob)) *
                        log2(error_prob / (1.0 - error_prob));

    /* Approximate Q^{-1}(ε) using simple approximation */
    double t;
    if (target_error >= 0.5) {
        t = 0.0;
    } else if (target_error <= 1e-15) {
        t = 8.0;
    } else {
        /* Crude approx: Q^{-1}(x) ≈ sqrt(2) erfcinv(2x) */
        t = sqrt(2.0) * (1.0 - target_error * 1.8);
        if (t < 0.0) t = 0.0;
    }

    double rate = cap - sqrt(dispersion / block_len) * t;
    return rate > 0.0 ? rate : 0.0;
}

double channel_converse_bound(double rate, double capacity, int block_len) {
    /* Strong converse (approximate): for R > C, P_e → 1 as n → ∞.
     * Bound: P_e ≥ 1 - exp(-n E_r(R)) where E_r is the random coding exponent.
     * Simplified: Pe ≥ 1 - C/R - 1/(nR) [Cover & Thomas, Sec 7.9] */
    if (block_len <= 0 || rate <= 0.0) return 1.0;
    if (rate <= capacity) return 0.0;

    double bound = 1.0 - capacity / rate - 1.0 / (block_len * rate);
    return bound > 0.0 ? bound : 0.0;
}

double random_coding_error_bound(double rate, double capacity, int block_length) {
    /* Random coding union bound: P_e ≤ 2^{-n E_r(R)}
     * where E_r(R) = max_{0≤ρ≤1} [E₀(ρ) - ρR] is the random coding exponent.
     * For BSC(p): E₀(ρ) = ρ log₂ 2 - (1+ρ) log₂(p^{1/(1+ρ)} + (1-p)^{1/(1+ρ)}). */
    if (rate >= capacity || block_length <= 0) return 1.0;

    /* Approximate E_r for rates near capacity */
    double gap = capacity - rate;
    double exponent = gap * gap / (2.0 * capacity) * block_length;
    double bound = pow(2.0, -exponent);
    return bound < 1.0 ? bound : 1.0;
}
