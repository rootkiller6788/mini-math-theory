#ifndef CHANNEL_H
#define CHANNEL_H

#include <stdbool.h>

/* ── L1: Core Channel Types ────────────────────────────────────── */
/** L1: Binary Symmetric Channel (BSC) — Cover & Thomas, Sec 7.1 */
typedef struct {
    double error_prob;       /* ε: bit flip probability */
    bool* input;
    bool* output;
    int length;
} BinarySymmetricChannel;

/** L1: Binary Erasure Channel (BEC) — Cover & Thomas, Sec 7.1.5 */
typedef struct {
    double erasure_prob;     /* α: probability of erasure */
} BinaryErasureChannel;

/** L1: Z-Channel (asymmetric binary) — Cover & Thomas, Sec 7.1 */
typedef struct {
    double p_1to0;           /* P(Y=0 | X=1) — one-sided error */
} ZChannel;

/** L3: Additive White Gaussian Noise (AWGN) channel */
typedef struct {
    double noise_variance;   /* σ² = N₀/2 */
    double signal_power;     /* P — average transmit power */
} AWGNChannel;

/** L3: Fading channel (Rayleigh) */
typedef struct {
    double avg_snr;          /* average SNR */
    double* gains;           /* channel gains per use */
    int n_uses;
} FadingChannel;

/** L1: General discrete memoryless channel */
typedef struct {
    double** matrix;          /* transition matrix: P(output|input) */
    int input_size;
    int output_size;
} GeneralChannel;

/** L3: Parallel Gaussian channels for water-filling */
typedef struct {
    double* noise_powers;     /* N_i for each subchannel */
    int n_subchannels;
    double total_power;       /* P_total constraint */
} ParallelChannels;

/* ── L1: BSC API ───────────────────────────────────────────────── */
void   bsc_init(BinarySymmetricChannel* ch, double error_prob);
double bsc_capacity(const BinarySymmetricChannel* ch);
void   bsc_transmit(BinarySymmetricChannel* ch, const bool* input, int len, bool* output);
double bsc_error_rate(const BinarySymmetricChannel* ch, const bool* original, int len);
double bsc_mutual_information(double error_prob, double input_dist);

/* ── L2: BEC API ──────────────────────────────────────────────── */
double bec_capacity(double erasure_prob);
void   bec_transmit(const bool* input, int len, double erasure_prob, int* output, bool* erased);
double bec_mutual_info(double erasure_prob, double input_dist);

/* ── L2: Z-Channel API ────────────────────────────────────────── */
double zchannel_capacity(double p_1to0);
double zchannel_mutual_info(double p_1to0, double input_dist);

/* ── L3: AWGN Channel ──────────────────────────────────────────── */
/** L3: Capacity: C = ½ log₂(1 + P/N) bits/channel use
 *  Cover & Thomas, Theorem 9.1.1 */
double awgn_capacity(double signal_power, double noise_variance);

/** L3: Simulate transmission: y = x + n, n ~ N(0, σ²) */
void   awgn_transmit(const double* x, int n, double noise_std, double* y);

/** L3: SNR in dB */
double snr_db(double signal_power, double noise_power);

/** L3: Bandwidth-normalized capacity: C = B log₂(1 + SNR) */
double awgn_capacity_bandwidth(double bandwidth, double signal_power, double noise_power);

/** L3: Capacity for complex baseband: C = log₂(1 + SNR) bits/s/Hz */
double awgn_capacity_spectral_efficiency(double snr_linear);

/* ── L5: Fading Channel ───────────────────────────────────────── */
/** L5: Ergodic capacity of Rayleigh fading: E[log₂(1 + |h|² SNR)] */
double fading_ergodic_capacity(double avg_snr, int n_samples);

/** L5: Outage probability: P(log₂(1+|h|² SNR) < R) */
double fading_outage_probability(double target_rate, double avg_snr, int n_samples);

/* ── L8: Water-Filling Power Allocation ───────────────────────── */
/** L8: Optimal power allocation for parallel Gaussian channels.
 *  P_i = max(0, μ - N_i) where μ chosen s.t. Σ P_i = P_total.
 *  Cover & Thomas, Theorem 9.4.1. Returns total capacity. */
double water_filling_capacity(const double* noise_powers, int n_subchannels,
                               double total_power, double* power_allocation);

/* ── L1: General Channel ───────────────────────────────────────── */
void   channel_init(GeneralChannel* ch, int in_size, int out_size);
void   channel_set_transition(GeneralChannel* ch, int input, int output, double prob);
double channel_capacity_blahut(GeneralChannel* ch, double epsilon, int max_iter);
void   channel_free(GeneralChannel* ch);
double channel_capacity_uniform_input(GeneralChannel* ch);

/** L2: Symmetric channel capacity (when the channel matrix is symmetric) */
bool   channel_is_symmetric(GeneralChannel* ch);
double channel_capacity_symmetric(GeneralChannel* ch);

/* ── L4: Channel Coding Theorem Verification ───────────────────── */
/** L4: Compute maximum reliable rate for BSC(p) = 1 - H(p).
 *  For rates below this, arbitrarily low error probability is possible. */
double channel_coding_theorem_rate_bound(double error_prob, double target_error, int block_len);

/** L4: Converse: Pe ≥ 1 - C/R - 1/(nR) (approximate strong converse) */
double channel_converse_bound(double rate, double capacity, int block_len);

/** L6: Simulate random coding bound approximation */
double random_coding_error_bound(double rate, double capacity, int block_length);

#endif
