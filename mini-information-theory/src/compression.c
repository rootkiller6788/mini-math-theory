#include "compression.h"
#include "entropy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CompressedData* compressed_create(int capacity) {
    CompressedData* cd = malloc(sizeof(CompressedData));
    cd->data = malloc(capacity);
    cd->size = 0;
    cd->capacity = capacity;
    return cd;
}

void compressed_free(CompressedData* cd) {
    if (cd) { free(cd->data); free(cd); }
}

void compressed_append(CompressedData* cd, unsigned char byte) {
    if (cd->size >= cd->capacity) {
        cd->capacity *= 2;
        cd->data = realloc(cd->data, cd->capacity);
    }
    cd->data[cd->size++] = byte;
}

CompressedData* rle_encode(const unsigned char* input, int len) {
    CompressedData* cd = compressed_create(256);
    if (len == 0) return cd;

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
    int max_len = cd->size * 128;
    unsigned char* out = malloc(max_len);
    int pos = 0;
    for (int i = 0; i < cd->size; i += 2) {
        int count = cd->data[i];
        unsigned char value = cd->data[i + 1];
        for (int j = 0; j < count; j++)
            out[pos++] = value;
    }
    *out_len = pos;
    return out;
}

#define LZW_DICT_SIZE 4096
#define LZW_INIT_DICT 256

CompressedData* lzw_encode(const unsigned char* input, int len) {
    CompressedData* cd = compressed_create(len / 2 + 256);
    unsigned char* dict_str[LZW_DICT_SIZE];
    int dict_len[LZW_DICT_SIZE];
    int dict_size = LZW_INIT_DICT;

    for (int i = 0; i < LZW_INIT_DICT; i++) {
        dict_str[i] = malloc(1);
        dict_str[i][0] = (unsigned char)i;
        dict_len[i] = 1;
    }

    if (len == 0) return cd;
    int w = input[0];
    int cur_len = 1;

    for (int i = 1; i < len; i++) {
        unsigned char k = input[i];
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
                memcpy(dict_str[dict_size], dict_str[w], cur_len);
                dict_str[dict_size][cur_len] = k;
                dict_len[dict_size] = cur_len + 1;
                dict_size++;
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
    unsigned char* out = malloc(cd->size * 8);
    int pos = 0;
    unsigned char* dict_str[LZW_DICT_SIZE];
    int dict_len[LZW_DICT_SIZE];
    int dict_size = LZW_INIT_DICT;

    for (int i = 0; i < LZW_INIT_DICT; i++) {
        dict_str[i] = malloc(1);
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
            memcpy(out + pos, dict_str[code], dict_len[code]);
            pos += dict_len[code];
            if (dict_size < LZW_DICT_SIZE) {
                dict_str[dict_size] = malloc(dict_len[prev] + 1);
                memcpy(dict_str[dict_size], dict_str[prev], dict_len[prev]);
                dict_str[dict_size][dict_len[prev]] = dict_str[code][0];
                dict_len[dict_size] = dict_len[prev] + 1;
                dict_size++;
            }
        } else if (code == dict_size) {
            memcpy(out + pos, dict_str[prev], dict_len[prev]);
            pos += dict_len[prev];
            out[pos++] = dict_str[prev][0];
            if (dict_size < LZW_DICT_SIZE) {
                dict_str[dict_size] = malloc(dict_len[prev] + 1);
                memcpy(dict_str[dict_size], dict_str[prev], dict_len[prev]);
                dict_str[dict_size][dict_len[prev]] = dict_str[prev][0];
                dict_len[dict_size] = dict_len[prev] + 1;
                dict_size++;
            }
        } else {
            break;
        }
        prev = code;
    }

    for (int i = 0; i < dict_size; i++) free(dict_str[i]);
    *out_len = pos;
    return out;
}

double compression_ratio(int original_size, int compressed_size) {
    return (double)original_size / compressed_size;
}

double compression_saving(int original_size, int compressed_size) {
    return 1.0 - (double)compressed_size / original_size;
}

double entropy_compression_bound(const char* text) {
    return entropy_from_counts(NULL, 0); // placeholder
}
