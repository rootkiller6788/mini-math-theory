#ifndef CODING_H
#define CODING_H

#include <stdbool.h>
#include "entropy.h"

#define CODE_MAX_SYMBOLS 256
#define CODE_MAX_LEN 256

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

double entropy_expected_length(const Codeword* codes, int n, const int* freqs, int total);
bool kraft_inequality_check(const Codeword* codes, int n);

HuffmanTree* huffman_build(const int* freqs, int n);
void huffman_generate_codes(HuffmanTree* tree, char* prefix, int plen, Codeword* codes, int* n);
void huffman_free_tree(HuffmanTree* t);

void shannon_fano(const SymbolProb* symbols, int n, Codeword* codes);
double coding_efficiency(const Codeword* codes, int n, const int* freqs, int total);

#endif
