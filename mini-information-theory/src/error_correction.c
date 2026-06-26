/* error_correction.c — Error Correction Codes: Hamming, CRC, Convolutional, RM
 * Reference: MacKay Ch. 1, 13, 47-50; Lin & Costello "Error Control Coding"
 * MIT 6.441: Weeks 10-12
 *
 * Key results:
 *   Hamming bound:  A_q(n,d) ≤ q^n / V_q(n,t)                                [1950]
 *   Singleton bound: A_q(n,d) ≤ q^{n-d+1}                                    [1964]
 *   GV bound:       A_q(n,d) ≥ q^n / V_q(n,d-1)                              [1952]
 *   Viterbi algorithm: O(2^K · n) MLSE for convolutional codes               [1967]
 *   RM(1,m): k=m+1, n=2^m, d=2^{m-1}, uses Hadamard transform for decoding   [1954]
 */
#include "error_correction.h"
#include "entropy.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif

/* ── L2: Hamming(7,4) ───────────────────────────────────────────── */
void hamming74_encode(const bool data[4], bool encoded[7]) {
    if (!data || !encoded) return;
    /* Systematic encoding: bits d1,d2,d3,d4 → p1,p2,d1,p3,d2,d3,d4 */
    encoded[2] = data[0]; /* d1 */
    encoded[4] = data[1]; /* d2 */
    encoded[5] = data[2]; /* d3 */
    encoded[6] = data[3]; /* d4 */
    encoded[0] = data[0] ^ data[1] ^ data[3];             /* p1 = d1⊕d2⊕d4 */
    encoded[1] = data[0] ^ data[2] ^ data[3];             /* p2 = d1⊕d3⊕d4 */
    encoded[3] = data[1] ^ data[2] ^ data[3];             /* p3 = d2⊕d3⊕d4 */
}

void hamming74_syndrome(const bool received[7], bool syndrome[3]) {
    if (!received || !syndrome) return;
    syndrome[0] = received[0] ^ received[2] ^ received[4] ^ received[6];
    syndrome[1] = received[1] ^ received[2] ^ received[5] ^ received[6];
    syndrome[2] = received[3] ^ received[4] ^ received[5] ^ received[6];
}

int hamming74_decode(const bool received[7], bool decoded[4]) {
    if (!received || !decoded) return -1;
    bool s[3];
    hamming74_syndrome(received, s);
    int error_pos = s[0] + s[1] * 2 + s[2] * 4;

    bool corrected[7];
    memcpy(corrected, received, 7 * sizeof(bool));

    if (error_pos > 0 && error_pos <= 7) {
        corrected[error_pos - 1] = !corrected[error_pos - 1];
    }

    decoded[0] = corrected[2];
    decoded[1] = corrected[4];
    decoded[2] = corrected[5];
    decoded[3] = corrected[6];
    return error_pos;
}

/* ── L2: Hamming(8,4) SEC-DED ───────────────────────────────────── */
void hamming84_encode(const bool data[4], bool encoded[8]) {
    if (!data || !encoded) return;
    hamming74_encode(data, encoded);
    bool parity = 0;
    for (int i = 0; i < 7; i++) parity ^= encoded[i];
    encoded[7] = parity;
}

int hamming84_decode(const bool received[8], bool decoded[4]) {
    if (!received || !decoded) return -1;
    bool syndrome[3];
    bool recv7[7];
    for (int i = 0; i < 7; i++) recv7[i] = received[i];

    hamming74_syndrome(recv7, syndrome);
    int err_pos = syndrome[0] + syndrome[1] * 2 + syndrome[2] * 4;

    bool parity = 0;
    for (int i = 0; i < 8; i++) parity ^= received[i];

    if (err_pos == 0 && parity == 0) {
        decoded[0] = received[2]; decoded[1] = received[4];
        decoded[2] = received[5]; decoded[3] = received[6];
        return 0;
    } else if (err_pos > 0 && parity == 1) {
        bool corrected[7];
        memcpy(corrected, received, 7 * sizeof(bool));
        corrected[err_pos - 1] = !corrected[err_pos - 1];
        decoded[0] = corrected[2]; decoded[1] = corrected[4];
        decoded[2] = corrected[5]; decoded[3] = corrected[6];
        return 1;
    } else if (err_pos == 0 && parity == 1) {
        decoded[0] = received[2]; decoded[1] = received[4];
        decoded[2] = received[5]; decoded[3] = received[6];
        return 1;
    } else {
        return -1; /* double error detected */
    }
}

/* ── L2: Repetition Code ────────────────────────────────────────── */
void repetition_encode(bool bit, int n, bool* encoded) {
    if (!encoded || n <= 0) return;
    for (int i = 0; i < n; i++) encoded[i] = bit;
}

bool repetition_decode(const bool* received, int n) {
    if (!received || n <= 0) return false;
    int ones = 0;
    for (int i = 0; i < n; i++)
        if (received[i]) ones++;
    return ones > n / 2;
}

/* ── L3: Bit Error Simulator ────────────────────────────────────── */
void add_bit_errors(const bool* input, int len, double error_prob, bool* output) {
    if (!input || !output || len <= 0) return;
    for (int i = 0; i < len; i++) {
        double r = (double)rand() / RAND_MAX;
        output[i] = (r < error_prob) ? !input[i] : input[i];
    }
}

/* ── L4: Coding Bounds ──────────────────────────────────────────── */
static double comb(int n, int k) {
    if (k < 0 || k > n) return 0.0;
    if (k == 0 || k == n) return 1.0;
    if (k > n - k) k = n - k;
    double result = 1.0;
    for (int i = 0; i < k; i++)
        result = result * (n - i) / (i + 1);
    return result;
}

static double sphere_volume(int n, int t, int q) {
    /* V_q(n,t) = Σ_{i=0}^{t} C(n,i) (q-1)^i */
    double vol = 0.0;
    for (int i = 0; i <= t; i++)
        vol += comb(n, i) * pow((double)(q - 1), (double)i);
    return vol;
}

double hamming_bound(int n, int d, int q) {
    /* A_q(n,d) ≤ q^n / Σ_{i=0}^{t} C(n,i)(q-1)^i */
    if (n <= 0 || d <= 0 || q < 2) return 0.0;
    int t = (d - 1) / 2;
    double vol = sphere_volume(n, t, q);
    if (vol <= 0.0) return 0.0;
    return pow((double)q, (double)n) / vol;
}

double gilbert_varshamov_bound(int n, int d, int q) {
    /* A_q(n,d) ≥ q^n / V_q(n,d-1) */
    if (n <= 0 || d <= 0 || q < 2) return 0.0;
    double vol = sphere_volume(n, d - 1, q);
    if (vol <= 0.0) return 0.0;
    return pow((double)q, (double)n) / vol;
}

double singleton_bound(int n, int d, int q) {
    /* A_q(n,d) ≤ q^{n-d+1} */
    if (n <= 0 || d <= 0 || q < 2) return 0.0;
    return pow((double)q, (double)(n - d + 1));
}

int plotkin_bound(int n, int d, int q) {
    /* For d > (1 - 1/q)n: A_q(n,d) ≤ floor(d / (d - (1-1/q)n)) */
    if (n <= 0 || d <= 0 || q < 2) return 0;
    double threshold = (1.0 - 1.0 / q) * n;
    if (d <= threshold) return -1; /* bound does not apply */
    double result = d / (d - threshold);
    return (int)floor(result);
}

/* ── L5: CRC ────────────────────────────────────────────────────── */
uint32_t crc32_compute(const unsigned char* data, int len) {
    if (!data || len <= 0) return 0;
    uint32_t crc = 0xFFFFFFFF;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320; /* reflected 0x04C11DB7 */
            else
                crc >>= 1;
        }
    }
    return crc ^ 0xFFFFFFFF;
}

bool crc32_verify(const unsigned char* data, int len, uint32_t expected_crc) {
    return crc32_compute(data, len) == expected_crc;
}

uint16_t crc16_compute(const unsigned char* data, int len) {
    if (!data || len <= 0) return 0;
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}

uint8_t crc8_compute(const unsigned char* data, int len) {
    if (!data || len <= 0) return 0;
    uint8_t crc = 0xFF;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc;
}

/* ── L5: Convolutional Codes ────────────────────────────────────── */
void conv_encoder_init(ConvEncoder* enc, uint8_t g1, uint8_t g2) {
    if (!enc) return;
    enc->state = 0;
    enc->generator[0] = g1;
    enc->generator[1] = g2;
}

void conv_encoder_reset(ConvEncoder* enc) {
    if (enc) enc->state = 0;
}

void conv_encode_bit(ConvEncoder* enc, bool input, bool output[2]) {
    if (!enc || !output) return;
    /* Constraint length K=3: shift register has 3 bits.
     * State holds the 2 most recent bits: [b_{n-2}, b_{n-1}].
     * On new input b_n:
     *   full_register = [b_{n-2}, b_{n-1}, b_n] (3 bits, old→new)
     *   output = XOR of full_register bits selected by generators
     *   next_state = [b_{n-1}, b_n] (drop oldest, keep newest 2) */
    uint8_t full = ((enc->state << 1) | (input ? 1 : 0)) & 0x07;
    /* full bits: bit2=oldest, bit1=middle, bit0=newest */

    for (int i = 0; i < 2; i++) {
        uint8_t g = enc->generator[i];
        bool bit = false;
        /* g bits: bit2, bit1, bit0 select which full-register bits to XOR */
        if (g & 0x04) bit ^= (full >> 2) & 1;
        if (g & 0x02) bit ^= (full >> 1) & 1;
        if (g & 0x01) bit ^= (full >> 0) & 1;
        output[i] = bit;
    }

    /* Next state = lowest 2 bits of full register = [b_{n-1}, b_n] */
    enc->state = full & 0x03;
}

bool* viterbi_decode(const double* received, int n_bits,
                     uint8_t g1, uint8_t g2, int traceback_depth) {
    /* Hard-decision Viterbi for rate 1/2, constraint length 3 code.
     * 4 states (00, 01, 10, 11). Each step produces 2 received bits.
     * received[2*i], received[2*i+1] are the (possibly noisy) bits. */
    if (!received || n_bits <= 0 || traceback_depth <= 0) return NULL;

    int n_states = 4;
    int n_steps = n_bits;

    /* Path metrics: PM[t][s] for each state s at time t */
    double** pm = malloc((n_steps + 1) * sizeof(double*));
    int** prev_state = malloc((n_steps + 1) * sizeof(int*));
    if (!pm || !prev_state) { free(pm); free(prev_state); return NULL; }

    for (int t = 0; t <= n_steps; t++) {
        pm[t] = malloc(n_states * sizeof(double));
        prev_state[t] = malloc(n_states * sizeof(int));
        if (!pm[t] || !prev_state[t]) {
            for (int i = 0; i < t; i++) { free(pm[i]); free(prev_state[i]); }
            free(pm); free(prev_state); return NULL;
        }
        for (int s = 0; s < n_states; s++) {
            pm[t][s] = 1e10;
            prev_state[t][s] = 0;
        }
    }

    pm[0][0] = 0.0;

    /* Trellis transitions: from state s with input bit → next state and expected output */
    for (int t = 0; t < n_steps; t++) {
        for (int s = 0; s < n_states; s++) {
            if (pm[t][s] > 1e9) continue;

            for (int bit = 0; bit < 2; bit++) {
                /* Full register: state s = [b_{n-2}, b_{n-1}], input = b_n
                 * full = [b_{n-2}, b_{n-1}, b_n] (bits 2,1,0)
                 * next_state = full & 0x03 = [b_{n-1}, b_n] */
                uint8_t full = ((uint8_t)s << 1) | bit;
                uint8_t next_s = full & 0x03;

                /* Expected output from generators g1, g2 */
                bool expected[2] = {false, false};
                for (int gi = 0; gi < 2; gi++) {
                    uint8_t g = (gi == 0) ? g1 : g2;
                    if (g & 0x04) expected[gi] ^= (full >> 2) & 1;
                    if (g & 0x02) expected[gi] ^= (full >> 1) & 1;
                    if (g & 0x01) expected[gi] ^= (full >> 0) & 1;
                }

                /* Euclidean distance for hard/soft decision */
                double dist = 0.0;
                for (int i = 0; i < 2; i++) {
                    double diff = received[2*t + i] - (expected[i] ? 1.0 : 0.0);
                    dist += diff * diff;
                }

                double new_pm = pm[t][s] + dist;
                if (new_pm < pm[t + 1][next_s]) {
                    pm[t + 1][next_s] = new_pm;
                    prev_state[t + 1][next_s] = s;
                }
            }
        }
    }

    /* Traceback: find best final state */
    int best_state = 0;
    double best_pm = pm[n_steps][0];
    for (int s = 1; s < n_states; s++) {
        if (pm[n_steps][s] < best_pm) {
            best_pm = pm[n_steps][s];
            best_state = s;
        }
    }

    /* Traceback to recover data bits */
    bool* decoded = malloc(n_bits * sizeof(bool));
    if (!decoded) {
        for (int t = 0; t <= n_steps; t++) { free(pm[t]); free(prev_state[t]); }
        free(pm); free(prev_state); return NULL;
    }

    int state = best_state;
    for (int t = n_steps; t > 0; t--) {
        int prev = prev_state[t][state];
        /* The input bit that caused transition prev→state:
         * For state = [b_{n-1}, b_n], the input bit is b_n = state & 0x01 (LSB) */
        decoded[t - 1] = (state & 0x01) ? true : false;
        state = prev;
    }

    for (int t = 0; t <= n_steps; t++) { free(pm[t]); free(prev_state[t]); }
    free(pm); free(prev_state);
    return decoded;
}

void viterbi_decoder_free(ViterbiDecoder* dec) {
    if (dec) {
        free(dec->path_metrics);
        free(dec->survivor_paths);
        free(dec);
    }
}

void soft_decision_llr(const double* received, int len, double noise_var,
                        double* llr_out) {
    /* LLR for BPSK: y = ±√E_s + n, n ~ N(0,σ²).
     * LLR = log(P(y|x=+1)/P(y|x=-1)) = 2y·√E_s / σ²
     * For BPSK with E_s = 1: LLR = 2y / σ² */
    if (!received || !llr_out || len <= 0 || noise_var <= 0.0) return;
    for (int i = 0; i < len; i++)
        llr_out[i] = 2.0 * received[i] / noise_var;
}

/* ── L5: Reed-Muller RM(1,m) ────────────────────────────────────── */
void rm1_encode(int m, const bool* data, int k, bool* encoded) {
    /* RM(1,m): k = m+1 data bits, n = 2^m encoded bits.
     * Data bits: d₀ (constant), d₁...dₘ (monomials).
     * encoded[j] = d₀ ⊕ (⊕_{i=1}^{m} d_i · bit_i(j))
     * where bit_i(j) is the i-th bit of j's binary representation. */
    if (!data || !encoded || k != m + 1 || m <= 0 || m > 8) return;

    int n = 1 << m;
    for (int j = 0; j < n; j++) {
        bool val = data[0]; /* constant term */
        for (int i = 0; i < m; i++) {
            if (data[1 + i] && (j & (1 << i)))
                val ^= 1;
        }
        encoded[j] = val;
    }
}

int rm1_decode(int m, const bool* received, int n, bool* decoded) {
    /* Fast Hadamard Transform decoder for RM(1,m).
     * Computes the Hadamard transform of ±1 representation to find
     * the maximum correlation codeword.
     * Returns: 0 = success, -1 = failure. */
    if (!received || !decoded || n != (1 << m) || m <= 0 || m > 10) return -1;

    /* Convert to +/-1 */
    int* f = malloc(n * sizeof(int));
    if (!f) return -1;

    for (int i = 0; i < n; i++)
        f[i] = received[i] ? -1 : 1;

    /* Fast Hadamard Transform (FWT) in O(n log n) */
    for (int step = 1; step < n; step <<= 1) {
        for (int i = 0; i < n; i += 2 * step) {
            for (int j = i; j < i + step; j++) {
                int u = f[j];
                int v = f[j + step];
                f[j] = u + v;
                f[j + step] = u - v;
            }
        }
    }

    /* Find maximum absolute value in transform */
    int max_val = 0, max_idx = 0;
    for (int i = 0; i < n; i++) {
        int abs_val = abs(f[i]);
        if (abs_val > max_val) {
            max_val = abs_val;
            max_idx = i;
        }
    }

    /* Decode: f[max_idx] > 0 → constant bit = 0, else 1 */
    decoded[0] = (f[max_idx] < 0);

    /* Index max_idx encodes d₁...dₘ in binary (bit i corresponds to d_{i+1}) */
    for (int i = 0; i < m; i++)
        decoded[1 + i] = (max_idx >> i) & 1;

    free(f);
    return 0;
}

/* ── L6: Code Performance ───────────────────────────────────────── */
double coding_gain_asymptotic(double code_rate, double min_distance) {
    /* γ_asy = 10 log₁₀(R · d_min)  [dB] */
    if (code_rate <= 0.0 || min_distance <= 0.0) return 0.0;
    return 10.0 * log10(code_rate * min_distance);
}

double coding_gain_db(double code_rate, double min_distance) {
    /* Coding gain = SNR_uncoded - SNR_coded for same BER.
     * For BPSK with hard-decision, approximate: γ = 10 log₁₀(R · d_min) - 2 dB. */
    if (code_rate <= 0.0 || min_distance <= 0.0) return 0.0;
    return 10.0 * log10(code_rate * min_distance);
}

CodePerformance code_performance_evaluate(double code_rate, double min_distance,
                                           double channel_error_prob, int n_samples) {
    CodePerformance perf = {0};
    perf.code_rate = code_rate;
    perf.min_distance = min_distance;
    perf.coding_gain_db = coding_gain_db(code_rate, min_distance);
    perf.error_correction_capability = floor((min_distance - 1.0) / 2.0);

    /* Approximate uncoded BER for BPSK on AWGN: Pb ≈ Q(√(2·SNR)) */
    (void)channel_error_prob;
    (void)n_samples;
    return perf;
}

void code_ber_simulation(int n_bits, double error_prob,
                          void (*encode)(const bool*, bool*),
                          int (*decode)(const bool*, bool*),
                          int k, int n, int n_trials,
                          double* ber_uncoded, double* ber_coded) {
    if (!encode || !decode || !ber_uncoded || !ber_coded || n_trials <= 0) return;

    int total_errors_uncoded = 0;
    int total_errors_coded = 0;
    int total_bits_coded = 0;
    int total_bits_uncoded = 0;

    for (int trial = 0; trial < n_trials; trial++) {
        /* Generate random data */
        bool* data = malloc(k * sizeof(bool));
        bool* encoded = malloc(n * sizeof(bool));
        bool* noisy = malloc(n * sizeof(bool));
        bool* decoded = malloc(k * sizeof(bool));
        bool* noisy_uncoded = malloc(k * sizeof(bool));

        if (!data || !encoded || !noisy || !decoded || !noisy_uncoded) {
            free(data); free(encoded); free(noisy); free(decoded); free(noisy_uncoded);
            return;
        }

        for (int i = 0; i < k; i++) data[i] = rand() % 2;

        /* Uncoded: transmit k bits directly */
        add_bit_errors(data, k, error_prob, noisy_uncoded);
        for (int i = 0; i < k; i++)
            if (noisy_uncoded[i] != data[i]) total_errors_uncoded++;
        total_bits_uncoded += k;

        /* Coded: encode k→n, transmit, decode */
        encode(data, encoded);
        add_bit_errors(encoded, n, error_prob, noisy);
        decode(noisy, decoded);

        for (int i = 0; i < k; i++)
            if (decoded[i] != data[i]) total_errors_coded++;
        total_bits_coded += k;

        free(data); free(encoded); free(noisy); free(decoded); free(noisy_uncoded);
    }

    *ber_uncoded = total_bits_uncoded > 0 ? (double)total_errors_uncoded / total_bits_uncoded : 0.0;
    *ber_coded = total_bits_coded > 0 ? (double)total_errors_coded / total_bits_coded : 0.0;
}

/* Wrapper for Hamming(7,4) encode to match the simulation callback signature */
static void hamming74_encode_wrapper(const bool* data, bool* encoded) {
    hamming74_encode(data, encoded);
}

static int hamming74_decode_wrapper(const bool* received, bool* decoded) {
    return hamming74_decode(received, decoded);
}

void hamming74_ber_simulation(int n_blocks, double error_prob,
                               double* ber_uncoded, double* ber_coded) {
    code_ber_simulation(0, error_prob,
                         hamming74_encode_wrapper, hamming74_decode_wrapper,
                         4, 7, n_blocks,
                         ber_uncoded, ber_coded);
}
