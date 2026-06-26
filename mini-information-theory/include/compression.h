#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <stdbool.h>

/* ── L1: Core Types ────────────────────────────────────────────── */
typedef struct {
    unsigned char* data;
    int size;
    int capacity;
} CompressedData;

/** L3: LZ77 match token */
typedef struct {
    int distance;    /* backwards distance to match (0 = literal) */
    int length;      /* length of match */
    unsigned char literal; /* literal byte when distance == 0 */
} LZ77Token;

/** L3: LZ77 compressed stream */
typedef struct {
    LZ77Token* tokens;
    int n_tokens;
} LZ77Stream;

/** L6: Compression statistics for benchmarking */
typedef struct {
    int original_size;
    int compressed_size;
    double ratio;
    double saving_pct;
    double entropy_lower_bound;
    double overhead_vs_entropy;
    const char* algorithm_name;
} CompressionStats;

/* ── L1: Container API ─────────────────────────────────────────── */
CompressedData* compressed_create(int capacity);
void compressed_free(CompressedData* cd);
void compressed_append(CompressedData* cd, unsigned char byte);

/* ── L2: Run-Length Encoding ───────────────────────────────────── */
CompressedData* rle_encode(const unsigned char* input, int len);
unsigned char*  rle_decode(CompressedData* cd, int* out_len);

/* ── L3: LZW ───────────────────────────────────────────────────── */
CompressedData* lzw_encode(const unsigned char* input, int len);
unsigned char*  lzw_decode(CompressedData* cd, int* out_len);

/* ── L5: LZ77 ──────────────────────────────────────────────────── */
/** L5: LZ77 (Ziv-Lempel 1977) — sliding window compression.
 *  window_size: look-back window for finding matches.
 *  lookahead_size: max match length to search for. */
LZ77Stream* lz77_encode(const unsigned char* input, int len,
                         int window_size, int lookahead_size);
unsigned char* lz77_decode(LZ77Stream* stream, int* out_len);
void lz77_stream_free(LZ77Stream* stream);

/** L5: Convert LZ77 tokens to compact binary format */
CompressedData* lz77_tokens_to_bytes(LZ77Stream* stream);
LZ77Stream*     lz77_bytes_to_tokens(CompressedData* cd);

/* ── L6: Full Compression Pipeline (bzip2-style) ───────────────── */
/** L6: BWT + MTF + RLE + Huffman — complete compression pipeline.
 *  Returns compressed data and populates stats. */
CompressedData* pipeline_compress(const unsigned char* input, int len,
                                   CompressionStats* stats);
unsigned char*  pipeline_decompress(CompressedData* cd, int* out_len);

/* ── L2: Compression Metrics ───────────────────────────────────── */
double compression_ratio(int original_size, int compressed_size);
double compression_saving(int original_size, int compressed_size);

/* ── L4: Entropy Compression Bound ─────────────────────────────── */
/** L4: Shannon's source coding theorem: minimum average code length
 *  L ≥ H(X). Returns the expected compressed size lower bound in bytes. */
double entropy_compression_bound(const char* text);

/** L4: Check if actual compression meets entropy bound.
 *  Returns (actual_size - entropy_bound), negative means below bound (impossible). */
double compression_bound_gap(int original_size, int compressed_size,
                              const char* text);

/** L2: Compression ratio benchmark on a text corpus */
CompressionStats compression_benchmark(const unsigned char* input, int len,
                                        const char* name);

#endif
