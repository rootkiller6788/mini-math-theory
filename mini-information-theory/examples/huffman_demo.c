#define CONTRACT_CHECK
#include "coding.h"
#include "compression.h"
#include "entropy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

int main(void) {
    printf("====== Huffman + Compression Demo ======\n\n");

    const char* text = "this is an example of a huffman tree for text compression";
    printf("Original text (%zu chars):\n  \"%s\"\n\n", strlen(text), text);

    double h = entropy_of_text(text);
    printf("Entropy: H(text) = %.4f bits/char\n", h);
    printf("Entropy lower bound: %.2f bytes (for %zu chars)\n\n",
           h * strlen(text) / 8.0, strlen(text));

    printf("--- 1. Huffman Coding ---\n");
    int freqs[256] = {0};
    for (int i = 0; text[i]; i++)
        freqs[(unsigned char)text[i]]++;

    HuffmanTree* tree = huffman_build(freqs, 256);
    Codeword codes[256];
    int ncodes = 0;
    char prefix[256];
    huffman_generate_codes(tree, prefix, 0, codes, &ncodes);

    int total = strlen(text);
    for (int i = 0; i < ncodes; i++)
        printf("  '%c' (freq=%d): %-10s (%d bits)\n",
               codes[i].symbol, freqs[codes[i].symbol],
               codes[i].code, codes[i].len);

    double avg_len = entropy_expected_length(codes, ncodes, freqs, total);
    printf("\nAvg code length: %.4f bits (vs entropy %.4f)\n", avg_len, h);
    printf("Efficiency: %.4f\n", coding_efficiency(codes, ncodes, freqs, total));

    double kraft_sum = 0.0;
    for (int i = 0; i < ncodes; i++)
        kraft_sum += pow(2.0, -(double)codes[i].len);
    bool kraft_ok = kraft_inequality_check(codes, ncodes);
    printf("Kraft inequality: sum 2^-L = %.6f (<= 1? %s)\n\n",
           kraft_sum, kraft_ok ? "YES" : "NO");

    huffman_free_tree(tree);

    printf("--- 2. RLE Compression ---\n");
    const char* rle_text = "AAABBBCCCCDDDDEEEEFFFFF";
    CompressedData* cd = rle_encode((const unsigned char*)rle_text, strlen(rle_text));
    printf("Original: \"%s\" (%zu bytes)\n", rle_text, strlen(rle_text));
    printf("RLE:      %d bytes (ratio %.2f:1, saving %.1f%%)\n",
           cd->size,
           compression_ratio(strlen(rle_text), cd->size),
           compression_saving(strlen(rle_text), cd->size) * 100);

    int dlen;
    unsigned char* dec = rle_decode(cd, &dlen);
    printf("Decoded:  \"%.*s\" (%d bytes)\n", dlen, dec, dlen);
    printf("Roundtrip: %s\n\n", memcmp(rle_text, dec, dlen) == 0 ? "OK ✓" : "FAIL");
    free(dec);
    compressed_free(cd);

    printf("--- 3. LZW Compression ---\n");
    const char* lzw_text = "TOBEORNOTTOBEORTOBEORNOT";
    CompressedData* lzw = lzw_encode((const unsigned char*)lzw_text, strlen(lzw_text));
    printf("Original: \"%s\" (%zu bytes)\n", lzw_text, strlen(lzw_text));
    printf("LZW:      %d bytes (ratio %.2f:1, saving %.1f%%)\n",
           lzw->size,
           compression_ratio(strlen(lzw_text), lzw->size),
           compression_saving(strlen(lzw_text), lzw->size) * 100);

    int lzw_dlen;
    unsigned char* lzw_dec = lzw_decode(lzw, &lzw_dlen);
    printf("Decoded:  \"%.*s\" (%d bytes)\n", lzw_dlen, lzw_dec, lzw_dlen);
    printf("Roundtrip: %s\n", memcmp(lzw_text, lzw_dec, lzw_dlen) == 0 ? "OK ✓" : "FAIL");
    free(lzw_dec);
    compressed_free(lzw);

    return 0;
}
