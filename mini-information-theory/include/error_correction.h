#ifndef ERROR_CORRECTION_H
#define ERROR_CORRECTION_H

#include <stdbool.h>
#include <stdint.h>

/* ── L1: Core Types ────────────────────────────────────────────── */
/** L3: Convolutional code state (rate 1/2, constraint length 3) */
typedef struct {
    uint8_t state;       /* shift register state (2 bits) */
    uint8_t generator[2]; /* generator polynomials */
} ConvEncoder;

/** L3: Viterbi decoder trellis */
typedef struct {
    double* path_metrics;
    int*    survivor_paths;  /* store decision bits */
    int     n_states;
    int     traceback_len;
} ViterbiDecoder;

/** L4: Code performance statistics */
typedef struct {
    double code_rate;         /* R = k/n */
    double min_distance;      /* d_min */
    double coding_gain_db;    /* asymptotic coding gain */
    double ber_uncoded;       /* bit error rate without coding */
    double ber_coded;         /* bit error rate with coding */
    double error_correction_capability; /* t = floor((d_min-1)/2) */
} CodePerformance;

/* ── L2: Hamming Codes ─────────────────────────────────────────── */
void hamming74_encode(const bool data[4], bool encoded[7]);
int  hamming74_decode(const bool received[7], bool decoded[4]);
void hamming74_syndrome(const bool received[7], bool syndrome[3]);

void hamming84_encode(const bool data[4], bool encoded[8]);
int  hamming84_decode(const bool received[8], bool decoded[4]);

/* ── L2: Repetition Code ────────────────────────────────────────── */
void repetition_encode(bool bit, int n, bool* encoded);
bool repetition_decode(const bool* received, int n);

/* ── L3: Bit Error Simulation ──────────────────────────────────── */
void add_bit_errors(const bool* input, int len, double error_prob, bool* output);

/* ── L4: Hamming Bound / Gilbert-Varshamov ──────────────────────── */
/** L4: Hamming (sphere-packing) bound:
 *  A_q(n,d) ≤ q^n / Σ_{i=0}^{t} C(n,i)(q-1)^i where t = ⌊(d-1)/2⌋ */
double hamming_bound(int n, int d, int q);

/** L4: Gilbert-Varshamov bound:
 *  A_q(n,d) ≥ q^n / Σ_{i=0}^{d-1} C(n,i)(q-1)^i */
double gilbert_varshamov_bound(int n, int d, int q);

/** L4: Singleton bound: A_q(n,d) ≤ q^{n-d+1} */
double singleton_bound(int n, int d, int q);

/** L4: Plotkin bound (for d > (1-1/q)n): A_q(n,d) ≤ ⌊ d / (d - (1-1/q)n) ⌋ */
int plotkin_bound(int n, int d, int q);

/* ── L5: CRC (Cyclic Redundancy Check) ─────────────────────────── */
/** L5: CRC-32 (IEEE 802.3 polynomial: 0x04C11DB7) */
uint32_t crc32_compute(const unsigned char* data, int len);
bool     crc32_verify(const unsigned char* data, int len, uint32_t expected_crc);

/** L5: CRC-16 (CCITT polynomial: 0x1021) */
uint16_t crc16_compute(const unsigned char* data, int len);

/** L5: CRC-8 (polynomial: 0x07) */
uint8_t crc8_compute(const unsigned char* data, int len);

/* ── L5: Convolutional Codes ───────────────────────────────────── */
/** L5: Initialize rate 1/2 convolutional encoder (generators in octal) */
void conv_encoder_init(ConvEncoder* enc, uint8_t g1, uint8_t g2);

/** L5: Encode one bit → 2 output bits (rate 1/2) */
void conv_encode_bit(ConvEncoder* enc, bool input, bool output[2]);

/** L5: Reset encoder state to 0 */
void conv_encoder_reset(ConvEncoder* enc);

/** L5: Viterbi decoder — maximum likelihood sequence estimation.
 *  received: array of received soft/hard bits (length = 2 * n_bits)
 *  n_bits: number of information bits encoded
 *  Returns decoded bits (caller must free). */
bool* viterbi_decode(const double* received, int n_bits,
                     uint8_t g1, uint8_t g2, int traceback_depth);

/** L5: Free Viterbi decoder resources */
void viterbi_decoder_free(ViterbiDecoder* dec);

/** L5: Soft-decision decoding: compute LLRs from AWGN samples */
void soft_decision_llr(const double* received, int len, double noise_var,
                        double* llr_out);

/* ── L5: Reed-Muller Codes ──────────────────────────────────────── */
/** L5: RM(1,m) first-order Reed-Muller code.
 *  k = 1 + m, n = 2^m, d_min = 2^{m-1}.
 *  Uses Hadamard transform for decoding. */
void   rm1_encode(int m, const bool* data, int k, bool* encoded);
int    rm1_decode(int m, const bool* received, int n, bool* decoded);

/* ── L6: Code Performance ───────────────────────────────────────── */
/** L6: Compute code performance metrics */
CodePerformance code_performance_evaluate(double code_rate, double min_distance,
                                           double channel_error_prob, int n_samples);

/** L6: Coding gain: SNR reduction (in dB) for same BER compared to uncoded BPSK */
double coding_gain_db(double code_rate, double min_distance);
double coding_gain_asymptotic(double code_rate, double min_distance);

/** L6: Simulate code performance on BSC */
void code_ber_simulation(int n_bits, double error_prob,
                          void (*encode)(const bool*, bool*),
                          int (*decode)(const bool*, bool*),
                          int k, int n, int n_trials,
                          double* ber_uncoded, double* ber_coded);

/** L6: Batch error simulation for Hamming(7,4) */
void hamming74_ber_simulation(int n_blocks, double error_prob,
                               double* ber_uncoded, double* ber_coded);

#endif
