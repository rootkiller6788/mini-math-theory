#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <stdbool.h>

typedef struct {
    unsigned char* data;
    int size;
    int capacity;
} CompressedData;

CompressedData* compressed_create(int capacity);
void compressed_free(CompressedData* cd);
void compressed_append(CompressedData* cd, unsigned char byte);

CompressedData* rle_encode(const unsigned char* input, int len);
unsigned char* rle_decode(CompressedData* cd, int* out_len);

CompressedData* lzw_encode(const unsigned char* input, int len);
unsigned char* lzw_decode(CompressedData* cd, int* out_len);

double compression_ratio(int original_size, int compressed_size);
double compression_saving(int original_size, int compressed_size);

double entropy_compression_bound(const char* text);

#endif
