/* test_all.c — Comprehensive test suite for mini-information-theory
 * Covers L1-L8 with assert-based verification.
 */
#include "entropy.h"
#include "mutual_information.h"
#include "kl_divergence.h"
#include "coding.h"
#include "compression.h"
#include "channel.h"
#include "error_correction.h"
#include "rate_distortion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>

#define ASSERT_NEAR(a, b, eps) do { \
    if (fabs((a) - (b)) > (eps)) { \
        fprintf(stderr, "FAIL: %s:%d  |%g - %g| > %g\n", __FILE__, __LINE__, (double)(a), (double)(b), (eps)); \
        abort(); \
    } \
} while(0)

#define ASSERT_TRUE(x) do { \
    if (!(x)) { fprintf(stderr, "FAIL: %s:%d  %s is false\n", __FILE__, __LINE__, #x); abort(); } \
} while(0)

static int tests_run = 0, tests_passed = 0;
#define RUN_TEST(name) do { \
    printf("  %s ... ", #name); fflush(stdout); \
    name(); \
    printf("OK\n"); tests_run++; tests_passed++; \
} while(0)

/* ── L1-L2: Entropy ─────────────────────────────────────────────── */
static void test_entropy_fair_coin(void) {
    double fair[2] = {0.5, 0.5};
    ASSERT_NEAR(entropy_shannon(fair, 2), 1.0, 1e-10);
}

static void test_entropy_binary_max(void) {
    ASSERT_NEAR(entropy_binary(0.5), 1.0, 1e-10);
    ASSERT_NEAR(entropy_binary(0.0), 0.0, 1e-10);
    ASSERT_NEAR(entropy_binary(1.0), 0.0, 1e-10);
}

static void test_entropy_deterministic(void) {
    double det[3] = {1.0, 0.0, 0.0};
    ASSERT_NEAR(entropy_shannon(det, 3), 0.0, 1e-10);
}

static void test_entropy_uniform(void) {
    double uni[4];
    for (int i = 0; i < 4; i++) uni[i] = 0.25;
    ASSERT_NEAR(entropy_shannon(uni, 4), 2.0, 1e-10);
}

static void test_entropy_joint(void) {
    double* jp[2];
    double r0[2] = {0.25, 0.25};
    double r1[2] = {0.25, 0.25};
    jp[0] = r0; jp[1] = r1;
    ASSERT_NEAR(entropy_joint((const double**)jp, 2, 2), 2.0, 1e-10);
}

static void test_entropy_conditional_independent(void) {
    double* jp[2];
    double r0[2] = {0.25, 0.25};
    double r1[2] = {0.25, 0.25};
    jp[0] = r0; jp[1] = r1;
    ASSERT_NEAR(entropy_conditional((const double**)jp, 2, 2), 1.0, 1e-10);
}

static void test_entropy_of_text(void) {
    ASSERT_NEAR(entropy_of_text("aaaa"), 0.0, 1e-10);
    double h_ab = entropy_of_text("ab");
    ASSERT_NEAR(h_ab, 1.0, 1e-10);
}

static void test_entropy_chain_rule(void) {
    double* jp[2];
    double r0[2] = {0.3, 0.2};
    double r1[2] = {0.1, 0.4};
    jp[0] = r0; jp[1] = r1;
    double diff = entropy_chain_rule_verify((const double**)jp, 2, 2);
    ASSERT_NEAR(diff, 0.0, 1e-10);
}

static void test_entropy_max_uniform(void) {
    ASSERT_NEAR(entropy_max_uniform(256), 8.0, 1e-10);
    ASSERT_NEAR(entropy_max_uniform(4), 2.0, 1e-10);
}

static void test_entropy_concavity(void) {
    double p[3] = {0.5, 0.3, 0.2};
    double q[3] = {0.2, 0.3, 0.5};
    double diff = entropy_concavity_verify(p, q, 3, 0.5);
    ASSERT_TRUE(diff >= -1e-12);
}

static void test_aep(void) {
    double probs[2] = {0.5, 0.5};
    int seq[10] = {0,1,0,1,0,1,0,1,0,1};
    double emp_h = aep_empirical_entropy(seq, 10, probs, 2);
    ASSERT_NEAR(emp_h, 1.0, 1e-10);
    ASSERT_TRUE(aep_is_typical(seq, 10, probs, 2, 0.2));
}

/* ── L2-L3: Mutual Information ──────────────────────────────────── */
static void test_mutual_info_independent(void) {
    double* pxy[2];
    double r0[2] = {0.25, 0.25};
    double r1[2] = {0.25, 0.25};
    pxy[0] = r0; pxy[1] = r1;
    double px[2] = {0.5, 0.5}, py[2] = {0.5, 0.5};
    ASSERT_NEAR(mutual_information((const double**)pxy, px, py, 2, 2), 0.0, 1e-10);
}

static void test_mutual_info_dependent(void) {
    double* pxy[2];
    double r0[2] = {0.4, 0.1};
    double r1[2] = {0.1, 0.4};
    pxy[0] = r0; pxy[1] = r1;
    double px[2] = {0.5, 0.5}, py[2] = {0.5, 0.5};
    double mi = mutual_information((const double**)pxy, px, py, 2, 2);
    ASSERT_TRUE(mi > 0.0);
}

static void test_mutual_info_non_negative(void) {
    double* pxy[2];
    double r0[2] = {0.3, 0.2};
    double r1[2] = {0.2, 0.3};
    pxy[0] = r0; pxy[1] = r1;
    double px[2] = {0.5, 0.5}, py[2] = {0.5, 0.5};
    ASSERT_TRUE(mutual_info_non_negative((const double**)pxy, px, py, 2, 2));
}

static void test_variation_of_information(void) {
    double* pxy[2];
    double r0[2] = {0.4, 0.1};
    double r1[2] = {0.1, 0.4};
    pxy[0] = r0; pxy[1] = r1;
    double px[2] = {0.5, 0.5}, py[2] = {0.5, 0.5};
    double vi = variation_of_information((const double**)pxy, px, py, 2, 2);
    ASSERT_TRUE(vi >= 0.0);
}

/* ── L3-L4: KL Divergence ───────────────────────────────────────── */
static void test_kl_non_negative(void) {
    double p[3] = {0.5, 0.3, 0.2};
    double q[3] = {0.2, 0.3, 0.5};
    ASSERT_TRUE(kl_non_negative(p, q, 3));
}

static void test_kl_self_zero(void) {
    double p[3] = {0.5, 0.3, 0.2};
    ASSERT_NEAR(kl_divergence(p, p, 3), 0.0, 1e-10);
}

static void test_cross_entropy_relation(void) {
    double p[3] = {0.5, 0.3, 0.2};
    double q[3] = {0.2, 0.3, 0.5};
    double ce = cross_entropy(p, q, 3);
    double h_p = entropy_shannon(p, 3);
    double d_kl = kl_divergence(p, q, 3);
    ASSERT_NEAR(ce, h_p + d_kl, 1e-10);
}

static void test_js_divergence(void) {
    double p[3] = {0.5, 0.3, 0.2};
    double q[3] = {0.2, 0.3, 0.5};
    double js = js_divergence(p, q, 3);
    ASSERT_TRUE(js >= 0.0);
    ASSERT_TRUE(js <= 1.0);
}

static void test_pinsker_inequality(void) {
    double p[3] = {0.5, 0.3, 0.2};
    double q[3] = {0.1, 0.4, 0.5};
    double verify = pinsker_inequality_verify(p, q, 3);
    ASSERT_TRUE(verify >= -1e-12);
}

static void test_kl_gaussian(void) {
    double d = kl_divergence_gaussian(0.0, 1.0, 0.0, 2.0);
    ASSERT_TRUE(d > 0.0);
    double d_self = kl_divergence_gaussian(0.0, 1.0, 0.0, 1.0);
    ASSERT_NEAR(d_self, 0.0, 1e-12);
}

/* ── L4-L5: Source Coding ───────────────────────────────────────── */
static void test_kraft_inequality(void) {
    Codeword codes[3];
    codes[0].len = 2; codes[1].len = 2; codes[2].len = 2;
    ASSERT_TRUE(kraft_inequality_check(codes, 3));
}

static void test_huffman_coding(void) {
    int freqs[256] = {0};
    freqs[(int)'a'] = 5; freqs[(int)'b'] = 2;
    freqs[(int)'c'] = 1; freqs[(int)'d'] = 1;
    HuffmanTree* tree = huffman_build(freqs, 256);
    ASSERT_TRUE(tree != NULL);

    Codeword codes[256];
    int ncodes = 0;
    char prefix[256];
    huffman_generate_codes(tree, prefix, 0, codes, &ncodes);
    ASSERT_TRUE(ncodes == 4);
    ASSERT_TRUE(kraft_inequality_check(codes, ncodes));

    int total = 9;
    double avg_len = entropy_expected_length(codes, ncodes, freqs, total);
    double h = entropy_from_counts(freqs, 256);
    ASSERT_TRUE(avg_len >= h - 1e-10);
    ASSERT_TRUE(avg_len < h + 1.0);

    huffman_free_tree(tree);
}

static void test_shannon_fano(void) {
    SymbolProb syms[4];
    syms[0] = (SymbolProb){0.4, 'a'};
    syms[1] = (SymbolProb){0.3, 'b'};
    syms[2] = (SymbolProb){0.2, 'c'};
    syms[3] = (SymbolProb){0.1, 'd'};
    Codeword codes[4];
    shannon_fano(syms, 4, codes);
    ASSERT_TRUE(kraft_inequality_check(codes, 4));
}

static void test_elias_gamma(void) {
    bool bits[64];
    int len = elias_gamma_encode(5, bits, 64);
    ASSERT_TRUE(len > 0);
    int consumed = 0;
    uint32_t val = elias_gamma_decode(bits, len, &consumed);
    ASSERT_TRUE(val == 5);
    ASSERT_TRUE(consumed == len);
}

static void test_elias_delta(void) {
    bool bits[64];
    int len = elias_delta_encode(100, bits, 64);
    ASSERT_TRUE(len > 0);
    int consumed;
    uint32_t val = elias_delta_decode(bits, len, &consumed);
    ASSERT_TRUE(val == 100);
}

/* ── L5: Compression ────────────────────────────────────────────── */
static void test_rle_roundtrip(void) {
    const unsigned char input[] = "AAABBBCCCC";
    CompressedData* cd = rle_encode(input, 10);
    ASSERT_TRUE(cd != NULL);
    int out_len;
    unsigned char* dec = rle_decode(cd, &out_len);
    ASSERT_TRUE(out_len == 10);
    ASSERT_TRUE(memcmp(input, dec, 10) == 0);
    free(dec);
    compressed_free(cd);
}

static void test_lzw_roundtrip(void) {
    const unsigned char input[] = "TOBEORNOTTOBEORTOBEORNOT";
    int len = strlen((const char*)input);
    CompressedData* cd = lzw_encode(input, len);
    ASSERT_TRUE(cd != NULL);
    int out_len;
    unsigned char* dec = lzw_decode(cd, &out_len);
    ASSERT_TRUE(out_len == len);
    ASSERT_TRUE(memcmp(input, dec, len) == 0);
    free(dec);
    compressed_free(cd);
}

static void test_compression_ratio(void) {
    ASSERT_NEAR(compression_ratio(100, 50), 2.0, 1e-10);
    ASSERT_NEAR(compression_saving(100, 30), 0.7, 1e-10);
}

static void test_entropy_bound(void) {
    const char* text = "aaaa";
    double bound = entropy_compression_bound(text);
    ASSERT_NEAR(bound, 0.0, 1e-10);
    double h = entropy_of_text("ab");
    double bound2 = entropy_compression_bound("ab");
    ASSERT_NEAR(bound2, h * 2.0 / 8.0, 1e-10);
}

static void test_lz77_roundtrip(void) {
    const unsigned char input[] = "ABRACADABRAABRACADABRA";
    int len = strlen((const char*)input);
    LZ77Stream* stream = lz77_encode(input, len, 32, 16);
    ASSERT_TRUE(stream != NULL);
    int out_len;
    unsigned char* dec = lz77_decode(stream, &out_len);
    ASSERT_TRUE(out_len == len);
    ASSERT_TRUE(memcmp(input, dec, len) == 0);
    free(dec);
    lz77_stream_free(stream);
}

static void test_mtf_roundtrip(void) {
    const unsigned char input[] = "banana";
    int len = 6;
    unsigned char encoded[6], decoded[6];
    mtf_encode(input, len, encoded);
    mtf_decode(encoded, len, decoded);
    ASSERT_TRUE(memcmp(input, decoded, len) == 0);
}

/* ── L5: Channel Models ─────────────────────────────────────────── */
static void test_bsc_capacity(void) {
    BinarySymmetricChannel bsc;
    bsc_init(&bsc, 0.0);
    ASSERT_NEAR(bsc_capacity(&bsc), 1.0, 1e-10);
    bsc_init(&bsc, 0.5);
    ASSERT_NEAR(bsc_capacity(&bsc), 0.0, 1e-10);
}

static void test_bec_capacity(void) {
    ASSERT_NEAR(bec_capacity(0.0), 1.0, 1e-10);
    ASSERT_NEAR(bec_capacity(1.0), 0.0, 1e-10);
    ASSERT_NEAR(bec_capacity(0.5), 0.5, 1e-10);
}

static void test_awgn_capacity(void) {
    double cap = awgn_capacity(1.0, 1.0);
    ASSERT_NEAR(cap, 0.5, 1e-10);
    double cap2 = awgn_capacity(3.0, 1.0);
    ASSERT_NEAR(cap2, 1.0, 1e-10);
}

static void test_blahut_arimoto(void) {
    GeneralChannel gc;
    channel_init(&gc, 2, 2);
    channel_set_transition(&gc, 0, 0, 0.9);
    channel_set_transition(&gc, 0, 1, 0.1);
    channel_set_transition(&gc, 1, 0, 0.1);
    channel_set_transition(&gc, 1, 1, 0.9);
    double cap = channel_capacity_blahut(&gc, 1e-6, 50);
    ASSERT_NEAR(cap, 1.0 - entropy_binary(0.1), 1e-6);
    channel_free(&gc);
}

static void test_water_filling(void) {
    double noises[3] = {1.0, 2.0, 4.0};
    double allocation[3];
    double cap = water_filling_capacity(noises, 3, 3.0, allocation);
    ASSERT_TRUE(cap > 0.0);
    double total = allocation[0] + allocation[1] + allocation[2];
    ASSERT_NEAR(total, 3.0, 1e-6);
}

/* ── L5: Error Correction ───────────────────────────────────────── */
static void test_hamming74_single_error(void) {
    bool data[4] = {1, 0, 1, 1};
    bool encoded[7];
    hamming74_encode(data, encoded);
    for (int err = 0; err < 7; err++) {
        bool received[7];
        memcpy(received, encoded, 7 * sizeof(bool));
        received[err] = !received[err];
        bool decoded[4];
        int result = hamming74_decode(received, decoded);
        ASSERT_TRUE(result == err + 1);
        ASSERT_TRUE(memcmp(decoded, data, 4) == 0);
    }
}

static void test_hamming84_single_error(void) {
    bool data[4] = {1, 0, 1, 1};
    bool encoded[8];
    hamming84_encode(data, encoded);
    bool received[8];
    memcpy(received, encoded, 8 * sizeof(bool));
    received[3] = !received[3];
    bool decoded[4];
    int result = hamming84_decode(received, decoded);
    ASSERT_TRUE(result >= 0);
    ASSERT_TRUE(memcmp(decoded, data, 4) == 0);
}

static void test_hamming84_double_error_detected(void) {
    bool data[4] = {1, 0, 1, 1};
    bool encoded[8];
    hamming84_encode(data, encoded);
    bool received[8];
    memcpy(received, encoded, 8 * sizeof(bool));
    received[0] = !received[0];
    received[5] = !received[5];
    bool decoded[4];
    int result = hamming84_decode(received, decoded);
    ASSERT_TRUE(result == -1);
}

static void test_crc32(void) {
    const unsigned char data[] = "123456789";
    uint32_t crc = crc32_compute(data, 9);
    ASSERT_TRUE(crc32_verify(data, 9, crc));
    ASSERT_TRUE(!crc32_verify(data, 9, crc ^ 1));
}

static void test_convolutional_code(void) {
    srand(42);
    ConvEncoder enc;
    conv_encoder_init(&enc, 0x07, 0x05);
    conv_encoder_reset(&enc);

    bool data[10];
    double received[20];
    for (int i = 0; i < 10; i++) {
        data[i] = rand() % 2;
        bool out[2];
        conv_encode_bit(&enc, data[i], out);
        received[2*i] = out[0] ? 1.0 : -1.0;
        received[2*i + 1] = out[1] ? 1.0 : -1.0;
    }

    bool* decoded = viterbi_decode(received, 10, 0x07, 0x05, 10);
    ASSERT_TRUE(decoded != NULL);
    ASSERT_TRUE(memcmp(decoded, data, 10) == 0);
    free(decoded);
}

static void test_hamming_bound(void) {
    double hb = hamming_bound(7, 3, 2);
    ASSERT_NEAR(hb, 16.0, 1e-10);
}

static void test_repetition_code(void) {
    bool encoded[3];
    repetition_encode(true, 3, encoded);
    bool received[3] = {true, false, true};
    ASSERT_TRUE(repetition_decode(received, 3) == true);
}

/* ── L8: Rate-Distortion ────────────────────────────────────────── */
static void test_rd_binary(void) {
    double r = rd_binary_hamming(0.5, 0.1);
    ASSERT_TRUE(r > 0.0);
    ASSERT_TRUE(r < 1.0);
    double r_zero = rd_binary_hamming(0.5, 0.0);
    ASSERT_NEAR(r_zero, 1.0, 1e-10);
    double r_max = rd_binary_hamming(0.5, 0.5);
    ASSERT_NEAR(r_max, 0.0, 1e-10);
}

static void test_rd_gaussian(void) {
    double r = rd_gaussian_squared_error(1.0, 0.25);
    ASSERT_NEAR(r, 1.0, 1e-10);
    double r_max = rd_gaussian_squared_error(1.0, 1.0);
    ASSERT_NEAR(r_max, 0.0, 1e-10);
}

/* ── L6: Pipeline ───────────────────────────────────────────────── */
static void test_pipeline_roundtrip(void) {
    const unsigned char input[] = "This is a test of the compression pipeline. "
                                   "It should handle repeated patterns well.";
    int len = strlen((const char*)input);
    CompressionStats stats;
    CompressedData* cd = pipeline_compress(input, len, &stats);
    ASSERT_TRUE(cd != NULL);
    ASSERT_TRUE(stats.compressed_size > 0);

    int out_len;
    unsigned char* dec = pipeline_decompress(cd, &out_len);
    ASSERT_TRUE(out_len == len);
    ASSERT_TRUE(memcmp(input, dec, len) == 0);
    free(dec);
    compressed_free(cd);
}

/* ── Main ───────────────────────────────────────────────────────── */
int main(void) {
    printf("=== mini-information-theory Test Suite ===\n\n");

    printf("L1-L2: Entropy\n");
    RUN_TEST(test_entropy_fair_coin);
    RUN_TEST(test_entropy_binary_max);
    RUN_TEST(test_entropy_deterministic);
    RUN_TEST(test_entropy_uniform);
    RUN_TEST(test_entropy_joint);
    RUN_TEST(test_entropy_conditional_independent);
    RUN_TEST(test_entropy_of_text);
    RUN_TEST(test_entropy_chain_rule);
    RUN_TEST(test_entropy_max_uniform);
    RUN_TEST(test_entropy_concavity);
    RUN_TEST(test_aep);

    printf("\nL2-L3: Mutual Information\n");
    RUN_TEST(test_mutual_info_independent);
    RUN_TEST(test_mutual_info_dependent);
    RUN_TEST(test_mutual_info_non_negative);
    RUN_TEST(test_variation_of_information);

    printf("\nL3-L4: KL Divergence\n");
    RUN_TEST(test_kl_non_negative);
    RUN_TEST(test_kl_self_zero);
    RUN_TEST(test_cross_entropy_relation);
    RUN_TEST(test_js_divergence);
    RUN_TEST(test_pinsker_inequality);
    RUN_TEST(test_kl_gaussian);

    printf("\nL4-L5: Source Coding\n");
    RUN_TEST(test_kraft_inequality);
    RUN_TEST(test_huffman_coding);
    RUN_TEST(test_shannon_fano);
    RUN_TEST(test_elias_gamma);
    RUN_TEST(test_elias_delta);

    printf("\nL5: Compression\n");
    RUN_TEST(test_rle_roundtrip);
    RUN_TEST(test_lzw_roundtrip);
    RUN_TEST(test_compression_ratio);
    RUN_TEST(test_entropy_bound);
    RUN_TEST(test_lz77_roundtrip);
    RUN_TEST(test_mtf_roundtrip);

    printf("\nL5: Channel Models\n");
    RUN_TEST(test_bsc_capacity);
    RUN_TEST(test_bec_capacity);
    RUN_TEST(test_awgn_capacity);
    RUN_TEST(test_blahut_arimoto);
    RUN_TEST(test_water_filling);

    printf("\nL5: Error Correction\n");
    RUN_TEST(test_hamming74_single_error);
    RUN_TEST(test_hamming84_single_error);
    RUN_TEST(test_hamming84_double_error_detected);
    RUN_TEST(test_crc32);
    RUN_TEST(test_convolutional_code);
    RUN_TEST(test_hamming_bound);
    RUN_TEST(test_repetition_code);

    printf("\nL8: Rate-Distortion\n");
    RUN_TEST(test_rd_binary);
    RUN_TEST(test_rd_gaussian);

    printf("\nL6: Pipeline\n");
    RUN_TEST(test_pipeline_roundtrip);

    printf("\n========================================\n");
    printf("ALL %d TESTS PASSED (%d/%d)\n", tests_run, tests_passed, tests_run);
    return 0;
}