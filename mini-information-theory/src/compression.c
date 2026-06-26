/* compression.c — Lossless Compression: RLE, LZW, LZ77, BWT Pipeline
 * Reference: Cover & Thomas, Ch. 5; Salomon "Data Compression" 4th Ed.
 * MIT 6.441: Weeks 5-7
 *
 * Key results:
 *   Shannon's source coding theorem: L ≥ H(X)     [Theorem 5.4.1]
 *   LZ77 achieves entropy rate for stationary ergodic sources [Ziv-Lempel 1977]
 *   LZW: Welch 1984, derivative of LZ78            [Ziv-Lempel 1978]
 *   BWT: Burrows-Wheeler 1994, basis of bzip2      [Burrows-Wheeler 1994]
 */
#include "compression.h"
#include "coding.h"
#include "entropy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ── L1: Container ──────────────────────────────────────────────── */
CompressedData* compressed_create(int capacity) {
    CompressedData* cd = malloc(sizeof(CompressedData));
    if (!cd) return NULL;
    cd->data = malloc(capacity > 0 ? capacity : 256);
    if (!cd->data) { free(cd); return NULL; }
    cd->size = 0;
    cd->capacity = capacity > 0 ? capacity : 256;
    return cd;
}

void compressed_free(CompressedData* cd) {
    if (cd) { free(cd->data); free(cd); }
}

void compressed_append(CompressedData* cd, unsigned char byte) {
    if (!cd) return;
    if (cd->size >= cd->capacity) {
        cd->capacity = cd->capacity * 2 + 1;
        unsigned char* tmp = realloc(cd->data, cd->capacity);
        if (!tmp) return;
        cd->data = tmp;
    }
    cd->data[cd->size++] = byte;
}

/* ── L2: RLE ────────────────────────────────────────────────────── */
CompressedData* rle_encode(const unsigned char* input, int len) {
    CompressedData* cd = compressed_create(len > 0 ? len : 256);
    if (!cd || len == 0) return cd;
    if (!input) { cd->size = 0; return cd; }

    int run = 1;
    for (int i = 1; i < len; i++) {
        if (input[i] == input[i - 1] && run < 255) {
            run++;
        } else {
            compressed_append(cd, (unsigned char)run);
            compressed_append(cd, input[i - 1]);
            run = 1;
        }
    }
    compressed_append(cd, (unsigned char)run);
    compressed_append(cd, input[len - 1]);
    return cd;
}

unsigned char* rle_decode(CompressedData* cd, int* out_len) {
    if (!cd || !out_len) { if (out_len) *out_len = 0; return NULL; }
    int max_len = cd->size * 128;
    unsigned char* out = malloc(max_len + 1);
    if (!out) { *out_len = 0; return NULL; }
    int pos = 0;
    for (int i = 0; i < cd->size; i += 2) {
        int count = cd->data[i];
        unsigned char value = cd->data[i + 1];
        for (int j = 0; j < count && pos < max_len; j++)
            out[pos++] = value;
    }
    *out_len = pos;
    return out;
}

/* ── L3: LZW ────────────────────────────────────────────────────── */
#define LZW_DICT_SIZE 4096
#define LZW_INIT_DICT 256

CompressedData* lzw_encode(const unsigned char* input, int len) {
    CompressedData* cd = compressed_create(len > 0 ? len / 2 + 256 : 256);
    if (!cd || !input || len == 0) return cd;

    unsigned char* dict_str[LZW_DICT_SIZE];
    int dict_len[LZW_DICT_SIZE];
    int dict_size = LZW_INIT_DICT;

    for (int i = 0; i < LZW_INIT_DICT; i++) {
        dict_str[i] = malloc(1);
        if (!dict_str[i]) {
            for (int j = 0; j < i; j++) free(dict_str[j]);
            return cd;
        }
        dict_str[i][0] = (unsigned char)i;
        dict_len[i] = 1;
    }

    if (len == 0) return cd;
    int w = input[0];
    int cur_len = 1;

    for (int ip = 1; ip < len; ip++) {
        unsigned char k = input[ip];

        /* Search for w + k in dictionary */
        int found = -1;
        for (int d = LZW_INIT_DICT; d < dict_size; d++) {
            if (dict_len[d] == cur_len + 1 &&
                memcmp(dict_str[d], dict_str[w], cur_len) == 0 &&
                dict_str[d][dict_len[d] - 1] == k) {
                found = d;
                break;
            }
        }

        if (found >= 0) {
            w = found;
            cur_len++;
        } else {
            compressed_append(cd, (unsigned char)(w >> 8));
            compressed_append(cd, (unsigned char)(w & 0xFF));

            if (dict_size < LZW_DICT_SIZE) {
                dict_str[dict_size] = malloc(cur_len + 1);
                if (dict_str[dict_size]) {
                    memcpy(dict_str[dict_size], dict_str[w], cur_len);
                    dict_str[dict_size][cur_len] = k;
                    dict_len[dict_size] = cur_len + 1;
                    dict_size++;
                }
            }
            w = k;
            cur_len = 1;
        }
    }
    compressed_append(cd, (unsigned char)(w >> 8));
    compressed_append(cd, (unsigned char)(w & 0xFF));

    for (int i = 0; i < LZW_INIT_DICT; i++) free(dict_str[i]);
    for (int i = LZW_INIT_DICT; i < dict_size; i++) free(dict_str[i]);
    return cd;
}

unsigned char* lzw_decode(CompressedData* cd, int* out_len) {
    if (!cd || !out_len) { if (out_len) *out_len = 0; return NULL; }
    unsigned char* out = malloc(cd->size * 8 + 1);
    if (!out) { *out_len = 0; return NULL; }
    int pos = 0;

    unsigned char* dict_str[LZW_DICT_SIZE];
    int dict_len[LZW_DICT_SIZE];
    int dict_size = LZW_INIT_DICT;

    for (int i = 0; i < LZW_INIT_DICT; i++) {
        dict_str[i] = malloc(1);
        if (!dict_str[i]) {
            for (int j = 0; j < i; j++) free(dict_str[j]);
            free(out); *out_len = 0; return NULL;
        }
        dict_str[i][0] = (unsigned char)i;
        dict_len[i] = 1;
    }

    if (cd->size < 2) { *out_len = 0; return out; }

    int prev = (cd->data[0] << 8) | cd->data[1];
    memcpy(out + pos, dict_str[prev], dict_len[prev]);
    pos += dict_len[prev];

    for (int i = 2; i < cd->size; i += 2) {
        int code = (cd->data[i] << 8) | cd->data[i + 1];

        if (code < dict_size) {
            int add_len = dict_len[code];
            memcpy(out + pos, dict_str[code], add_len);
            pos += add_len;

            if (dict_size < LZW_DICT_SIZE) {
                dict_str[dict_size] = malloc(dict_len[prev] + 1);
                if (dict_str[dict_size]) {
                    memcpy(dict_str[dict_size], dict_str[prev], dict_len[prev]);
                    dict_str[dict_size][dict_len[prev]] = dict_str[code][0];
                    dict_len[dict_size] = dict_len[prev] + 1;
                    dict_size++;
                }
            }
        } else if (code == dict_size) {
            memcpy(out + pos, dict_str[prev], dict_len[prev]);
            pos += dict_len[prev];
            out[pos++] = dict_str[prev][0];

            if (dict_size < LZW_DICT_SIZE) {
                dict_str[dict_size] = malloc(dict_len[prev] + 1);
                if (dict_str[dict_size]) {
                    memcpy(dict_str[dict_size], dict_str[prev], dict_len[prev]);
                    dict_str[dict_size][dict_len[prev]] = dict_str[prev][0];
                    dict_len[dict_size] = dict_len[prev] + 1;
                    dict_size++;
                }
            }
        }
        prev = code;
    }

    for (int i = 0; i < dict_size; i++) free(dict_str[i]);
    *out_len = pos;
    return out;
}

/* ── L5: LZ77 ───────────────────────────────────────────────────── */
LZ77Stream* lz77_encode(const unsigned char* input, int len,
                         int window_size, int lookahead_size) {
    if (!input || len <= 0) return NULL;

    int max_tokens = len;
    LZ77Stream* stream = malloc(sizeof(LZ77Stream));
    if (!stream) return NULL;
    stream->tokens = malloc(max_tokens * sizeof(LZ77Token));
    if (!stream->tokens) { free(stream); return NULL; }
    stream->n_tokens = 0;

    int pos = 0;
    while (pos < len) {
        int best_len = 0, best_dist = 0;

        int search_start = pos - window_size;
        if (search_start < 0) search_start = 0;

        for (int i = search_start; i < pos; i++) {
            int match_len = 0;
            while (match_len < lookahead_size &&
                   pos + match_len < len &&
                   input[i + match_len] == input[pos + match_len])
                match_len++;

            if (match_len > best_len) {
                best_len = match_len;
                best_dist = pos - i;
            }
        }

        if (best_len >= 3 && stream->n_tokens < max_tokens) {
            stream->tokens[stream->n_tokens].distance = best_dist;
            stream->tokens[stream->n_tokens].length = best_len;
            stream->tokens[stream->n_tokens].literal = 0;
            stream->n_tokens++;
            pos += best_len;
        } else {
            if (stream->n_tokens < max_tokens) {
                stream->tokens[stream->n_tokens].distance = 0;
                stream->tokens[stream->n_tokens].length = 1;
                stream->tokens[stream->n_tokens].literal = input[pos];
                stream->n_tokens++;
            }
            pos++;
        }
    }
    return stream;
}

unsigned char* lz77_decode(LZ77Stream* stream, int* out_len) {
    if (!stream || !out_len) { if (out_len) *out_len = 0; return NULL; }

    int max_len = stream->n_tokens * 258 + 256;
    unsigned char* out = malloc(max_len + 1);
    if (!out) { *out_len = 0; return NULL; }

    int pos = 0;
    for (int t = 0; t < stream->n_tokens; t++) {
        if (stream->tokens[t].distance == 0) {
            out[pos++] = stream->tokens[t].literal;
        } else {
            int start = pos - stream->tokens[t].distance;
            for (int i = 0; i < stream->tokens[t].length && pos < max_len; i++)
                out[pos++] = out[start + i];
        }
    }
    *out_len = pos;
    return out;
}

void lz77_stream_free(LZ77Stream* stream) {
    if (stream) { free(stream->tokens); free(stream); }
}

CompressedData* lz77_tokens_to_bytes(LZ77Stream* stream) {
    if (!stream) return NULL;
    CompressedData* cd = compressed_create(stream->n_tokens * 3 + 4);
    if (!cd) return NULL;

    /* Header: number of tokens */
    compressed_append(cd, (unsigned char)(stream->n_tokens >> 24));
    compressed_append(cd, (unsigned char)(stream->n_tokens >> 16));
    compressed_append(cd, (unsigned char)(stream->n_tokens >> 8));
    compressed_append(cd, (unsigned char)(stream->n_tokens));

    for (int i = 0; i < stream->n_tokens; i++) {
        LZ77Token* t = &stream->tokens[i];
        if (t->distance == 0) {
            compressed_append(cd, 0); /* flag: literal */
            compressed_append(cd, t->literal);
        } else {
            unsigned char flag = (t->distance > 255) ? 2 : 1;
            compressed_append(cd, flag);
            if (flag == 2) {
                compressed_append(cd, (unsigned char)(t->distance >> 8));
                compressed_append(cd, (unsigned char)(t->distance & 0xFF));
            } else {
                compressed_append(cd, (unsigned char)t->distance);
            }
            compressed_append(cd, (unsigned char)t->length);
        }
    }
    return cd;
}

LZ77Stream* lz77_bytes_to_tokens(CompressedData* cd) {
    if (!cd || cd->size < 4) return NULL;

    int n_tokens = (cd->data[0] << 24) | (cd->data[1] << 16) |
                   (cd->data[2] << 8)  | cd->data[3];

    LZ77Stream* stream = malloc(sizeof(LZ77Stream));
    if (!stream) return NULL;
    stream->tokens = malloc(n_tokens * sizeof(LZ77Token));
    if (!stream->tokens) { free(stream); return NULL; }
    stream->n_tokens = 0;

    int idx = 4;
    while (idx < cd->size && stream->n_tokens < n_tokens) {
        unsigned char flag = cd->data[idx++];
        if (flag == 0) {
            stream->tokens[stream->n_tokens].distance = 0;
            stream->tokens[stream->n_tokens].length = 1;
            stream->tokens[stream->n_tokens].literal = cd->data[idx++];
        } else {
            int dist;
            if (flag == 2) {
                dist = (cd->data[idx] << 8) | cd->data[idx + 1];
                idx += 2;
            } else {
                dist = cd->data[idx++];
            }
            int length = cd->data[idx++];
            stream->tokens[stream->n_tokens].distance = dist;
            stream->tokens[stream->n_tokens].length = length;
            stream->tokens[stream->n_tokens].literal = 0;
        }
        stream->n_tokens++;
    }
    return stream;
}

/* ── L6: Full Pipeline (BWT + MTF + RLE + Huffman) ──────────────── */
CompressedData* pipeline_compress(const unsigned char* input, int len,
                                   CompressionStats* stats) {
    if (!input || len <= 0) return compressed_create(64);

    /* Stage 1: BWT */
    unsigned char* bwt_out = malloc(len);
    if (!bwt_out) return NULL;
    int primary = bwt_encode(input, len, bwt_out);
    if (primary < 0) { free(bwt_out); return NULL; }

    /* Stage 2: MTF */
    unsigned char* mtf_out = malloc(len);
    if (!mtf_out) { free(bwt_out); return NULL; }
    mtf_encode(bwt_out, len, mtf_out);
    free(bwt_out);

    /* Stage 3: RLE on MTF output */
    CompressedData* rle_cd = rle_encode(mtf_out, len);
    free(mtf_out);
    if (!rle_cd) return NULL;

    /* Stage 4: Pack with primary index */
    CompressedData* final = compressed_create(rle_cd->size + 8);
    if (!final) { compressed_free(rle_cd); return NULL; }

    /* Header: original length + BWT primary index */
    compressed_append(final, (unsigned char)(len >> 24));
    compressed_append(final, (unsigned char)(len >> 16));
    compressed_append(final, (unsigned char)(len >> 8));
    compressed_append(final, (unsigned char)len);
    compressed_append(final, (unsigned char)(primary >> 24));
    compressed_append(final, (unsigned char)(primary >> 16));
    compressed_append(final, (unsigned char)(primary >> 8));
    compressed_append(final, (unsigned char)primary);

    for (int i = 0; i < rle_cd->size; i++)
        compressed_append(final, rle_cd->data[i]);

    compressed_free(rle_cd);

    if (stats) {
        stats->original_size = len;
        stats->compressed_size = final->size;
        stats->ratio = len > 0 ? (double)len / final->size : 0.0;
        stats->saving_pct = len > 0 ? (1.0 - (double)final->size / len) * 100.0 : 0.0;
        stats->algorithm_name = "BWT+MTF+RLE";
    }

    return final;
}

unsigned char* pipeline_decompress(CompressedData* cd, int* out_len) {
    if (!cd || !out_len || cd->size < 8) {
        if (out_len) *out_len = 0;
        return NULL;
    }

    int orig_len = (cd->data[0] << 24) | (cd->data[1] << 16) |
                   (cd->data[2] << 8)  | cd->data[3];
    int primary = (cd->data[4] << 24) | (cd->data[5] << 16) |
                  (cd->data[6] << 8)  | cd->data[7];

    /* Extract RLE data */
    CompressedData* rle_cd = compressed_create(cd->size - 8);
    if (!rle_cd) { *out_len = 0; return NULL; }
    for (int i = 8; i < cd->size; i++)
        compressed_append(rle_cd, cd->data[i]);

    /* RLE decode */
    int mtf_len;
    unsigned char* mtf_out = rle_decode(rle_cd, &mtf_len);
    compressed_free(rle_cd);

    if (!mtf_out) { *out_len = 0; return NULL; }

    /* MTF decode */
    unsigned char* bwt_in = malloc(mtf_len);
    if (!bwt_in) { free(mtf_out); *out_len = 0; return NULL; }
    mtf_decode(mtf_out, mtf_len, bwt_in);
    free(mtf_out);

    /* BWT decode */
    unsigned char* out = malloc(orig_len + 1);
    if (!out) { free(bwt_in); *out_len = 0; return NULL; }
    bwt_decode(primary, bwt_in, mtf_len, out);
    free(bwt_in);

    *out_len = orig_len;
    return out;
}

/* ── L2: Compression Metrics ────────────────────────────────────── */
double compression_ratio(int original_size, int compressed_size) {
    if (original_size <= 0 || compressed_size <= 0) return 0.0;
    return (double)original_size / compressed_size;
}

double compression_saving(int original_size, int compressed_size) {
    if (original_size <= 0) return 0.0;
    return 1.0 - (double)compressed_size / original_size;
}

/* ── L4: Entropy Compression Bound ──────────────────────────────── */
double entropy_compression_bound(const char* text) {
    /* Shannon's source coding theorem: L_min = H(X) bits per symbol.
     * Total compressed bytes ≥ n * H(X) / 8.
     * Returns the lower bound in bytes. */
    if (!text) return 0.0;
    double h = entropy_of_text(text);
    return h * strlen(text) / 8.0;
}

double compression_bound_gap(int original_size, int compressed_size,
                              const char* text) {
    /* Returns compressed_size - entropy_bound. Positive = above bound (OK). */
    (void)original_size;
    double bound = entropy_compression_bound(text);
    return (double)compressed_size - bound;
}

CompressionStats compression_benchmark(const unsigned char* input, int len,
                                        const char* name) {
    CompressionStats stats = {0};
    if (!input || len <= 0) return stats;

    stats.original_size = len;
    stats.algorithm_name = name ? name : "unknown";

    /* Compute entropy bound on raw text (assumes ASCII-ish input) */
    char* text = malloc(len + 1);
    if (text) {
        memcpy(text, input, len);
        text[len] = '\0';
        stats.entropy_lower_bound = entropy_compression_bound(text);
        free(text);
    }

    /* Compress with LZW */
    CompressedData* cd = lzw_encode(input, len);
    if (cd) {
        stats.compressed_size = cd->size;
        stats.ratio = compression_ratio(len, cd->size);
        stats.saving_pct = compression_saving(len, cd->size) * 100.0;
        stats.overhead_vs_entropy = cd->size - stats.entropy_lower_bound;
        compressed_free(cd);
    }

    return stats;
}
