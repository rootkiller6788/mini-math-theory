#ifndef CODING_H
#define CODING_H

#include <stdbool.h>
#include <stdint.h>
#include "entropy.h"

#define CODE_MAX_SYMBOLS 256
#define CODE_MAX_LEN 256

/* ── L1: Core Types ────────────────────────────────────────────── */
typedef struct {
    char code[CODE_MAX_LEN];
    int len;
    unsigned char symbol;
} Codeword;

typedef struct HuffmanTree {
    unsigned char symbol;
    int freq;
    struct HuffmanTree *left, *right;
} HuffmanTree;

/** L3: Arithmetic coding state machine */
typedef struct {
    uint32_t low;       /* lower bound of current interval */
    uint32_t high;      /* upper bound of current interval */
    uint32_t pending;   /* pending bits for underflow handling */
    uint32_t* output;   /* output bit buffer */
    int out_pos;
    int out_capacity;
} ArithmeticEncoder;

typedef struct {
    uint32_t low;
    uint32_t high;
    uint32_t value;     /* current tag value */
    uint32_t* input;    /* input bit stream */
    int in_pos;
    int in_len;
} ArithmeticDecoder;

/** L3: Tunstall coding — fixed-length to variable-length */
typedef struct {
    unsigned char* pattern;
    int pattern_len;
    unsigned char code;
} TunstallEntry;

/* ── L2: Huffman Coding ────────────────────────────────────────── */
HuffmanTree* huffman_build(const int* freqs, int n);
void huffman_generate_codes(HuffmanTree* tree, char* prefix, int plen, Codeword* codes, int* n);
void huffman_free_tree(HuffmanTree* t);

/* ── L2: Shannon-Fano Coding ───────────────────────────────────── */
void shannon_fano(const SymbolProb* symbols, int n, Codeword* codes);

/* ── L4: Kraft-McMillan Inequality ─────────────────────────────── */
bool kraft_inequality_check(const Codeword* codes, int n);

/* ── L4: Source Coding Theorem Verification ────────────────────── */
double entropy_expected_length(const Codeword* codes, int n, const int* freqs, int total);
double coding_efficiency(const Codeword* codes, int n, const int* freqs, int total);

/* ── L5: Arithmetic Coding ─────────────────────────────────────── */
/** L5: Initialize arithmetic encoder with output buffer capacity */
ArithmeticEncoder* arith_encoder_create(int capacity);
void                arith_encoder_free(ArithmeticEncoder* enc);

/** L5: Encode a symbol given cumulative probability interval [cum_low, cum_high)
 *  within total range [0, total). */
void arith_encode_symbol(ArithmeticEncoder* enc, uint32_t cum_low, uint32_t cum_high,
                          uint32_t total);

/** L5: Flush remaining bits to complete encoding */
void arith_encoder_flush(ArithmeticEncoder* enc);

/** L5: Adaptive probability model: update cumulative counts */
void arith_update_model(uint32_t* counts, int alphabet, int symbol);

/** L5: Initialize arithmetic decoder */
ArithmeticDecoder* arith_decoder_create(const uint32_t* bits, int nbits, int capacity);
void               arith_decoder_free(ArithmeticDecoder* dec);

/** L5: Decode next symbol from bitstream */
int arith_decode_symbol(ArithmeticDecoder* dec, const uint32_t* cum_counts, int alphabet,
                         uint32_t total);

/* ── L5: Elias Universal Codes ──────────────────────────────────── */
/** L5: Elias Gamma code: integer x ≥ 1 → floor(log₂ x) zeros + binary(x) */
int  elias_gamma_encode(uint32_t value, bool* bits, int max_bits);
uint32_t elias_gamma_decode(const bool* bits, int max_bits, int* consumed);

/** L5: Elias Delta code: Gamma(length) + binary(x without MSB) */
int  elias_delta_encode(uint32_t value, bool* bits, int max_bits);
uint32_t elias_delta_decode(const bool* bits, int max_bits, int* consumed);

/** L5: Elias Omega code (recursive length encoding) */
int  elias_omega_encode(uint32_t value, bool* bits, int max_bits);
uint32_t elias_omega_decode(const bool* bits, int max_bits, int* consumed);

/* ── L5: Golomb/Rice Coding ────────────────────────────────────── */
/** L5: Golomb code with parameter M. Optimal for geometric distribution. */
int  golomb_encode(uint32_t value, uint32_t m, bool* bits, int max_bits);
uint32_t golomb_decode(const bool* bits, int max_bits, uint32_t m, int* consumed);

/* ── L4: Sardinas-Patterson Unique Decodability Test ────────────── */
/** L4: Test whether a set of codewords is uniquely decodable.
 *  Sardinas-Patterson 1953 algorithm. O(n²·L) where n = #codewords, L = max length. */
bool sardinas_patterson_test(const Codeword* codes, int n);

/* ── L5: Tunstall Coding ───────────────────────────────────────── */
/** L5: Tunstall 1967 — parse input into variable-length blocks, assign fixed-length codes.
 *  Builds dictionary of size 2^k where k is the output bit width. */
int tunstall_build(const double* probs, int alphabet, int k, TunstallEntry* dict);

/* ── L2: Move-to-Front Transform (compression preprocessing) ────── */
void mtf_encode(const unsigned char* input, int len, unsigned char* output);
void mtf_decode(const unsigned char* input, int len, unsigned char* output);

/* ── L2: Burrows-Wheeler Transform ─────────────────────────────── */
/** L2: BWT — reversible permutation that groups similar characters.
 *  Used as preprocessing in bzip2. Returns primary index. */
int bwt_encode(const unsigned char* input, int len, unsigned char* output);
void bwt_decode(int primary_index, const unsigned char* encoded, int len, unsigned char* output);

/** L2: Suffix array comparison helper for BWT build */
int bwt_suffix_compare(const unsigned char* T, int n, int i, int j);

#endif
