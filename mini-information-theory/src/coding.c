#include "coding.h"
#include "entropy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

HuffmanTree* huffman_build(const int* freqs, int n) {
    HuffmanTree* nodes[CODE_MAX_SYMBOLS];
    int size = 0;

    for (int i = 0; i < n; i++) {
        if (freqs[i] > 0) {
            HuffmanTree* node = malloc(sizeof(HuffmanTree));
            node->symbol = (unsigned char)i;
            node->freq = freqs[i];
            node->left = node->right = NULL;
            nodes[size++] = node;
        }
    }

    while (size > 1) {
        int min1 = 0, min2 = 1;
        if (nodes[min1]->freq > nodes[min2]->freq) { int t = min1; min1 = min2; min2 = t; }
        for (int i = 2; i < size; i++) {
            if (nodes[i]->freq < nodes[min1]->freq) { min2 = min1; min1 = i; }
            else if (nodes[i]->freq < nodes[min2]->freq) { min2 = i; }
        }

        HuffmanTree* internal = malloc(sizeof(HuffmanTree));
        internal->symbol = 0;
        internal->freq = nodes[min1]->freq + nodes[min2]->freq;
        internal->left = nodes[min1];
        internal->right = nodes[min2];
        nodes[min1] = internal;
        nodes[min2] = nodes[size - 1];
        size--;
    }

    return size > 0 ? nodes[0] : NULL;
}

void huffman_generate_codes(HuffmanTree* tree, char* prefix, int plen,
                            Codeword* codes, int* n) {
    if (!tree) return;
    if (!tree->left && !tree->right) {
        codes[*n].symbol = tree->symbol;
        memcpy(codes[*n].code, prefix, plen);
        codes[*n].len = plen;
        (*n)++;
        return;
    }
    prefix[plen] = '0';
    huffman_generate_codes(tree->left, prefix, plen + 1, codes, n);
    prefix[plen] = '1';
    huffman_generate_codes(tree->right, prefix, plen + 1, codes, n);
}

void huffman_free_tree(HuffmanTree* t) {
    if (!t) return;
    huffman_free_tree(t->left);
    huffman_free_tree(t->right);
    free(t);
}

double entropy_expected_length(const Codeword* codes, int n, const int* freqs, int total) {
    if (total == 0) return 0.0;
    double avg = 0.0;
    for (int i = 0; i < n; i++) {
        double p = (double)freqs[codes[i].symbol] / total;
        avg += p * codes[i].len;
    }
    return avg;
}

bool kraft_inequality_check(const Codeword* codes, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++)
        sum += pow(2.0, -(double)codes[i].len);
    return sum <= 1.0 + 1e-10;
}

double coding_efficiency(const Codeword* codes, int n, const int* freqs, int total) {
    double el = entropy_expected_length(codes, n, freqs, total);
    double h = entropy_from_counts(freqs, CODE_MAX_SYMBOLS);
    return el > 0 ? h / el : 0.0;
}

void shannon_fano(const SymbolProb* symbols, int n, Codeword* codes) {
    typedef struct { SymbolProb* arr; int start; int end; char* prefix; int plen; } SFNode;
    SFNode stack[128];
    int top = 0;

    char init_prefix[256] = "";
    stack[top++] = (SFNode){(SymbolProb*)symbols, 0, n, init_prefix, 0};

    while (top > 0) {
        SFNode node = stack[--top];
        if (node.start >= node.end) continue;

        if (node.end - node.start == 1) {
            int idx = node.start;
            codes[idx].symbol = (unsigned char)node.arr[idx].symbol;
            memcpy(codes[idx].code, node.prefix, node.plen);
            codes[idx].len = node.plen;
            continue;
        }

        double total = 0.0;
        for (int i = node.start; i < node.end; i++) total += node.arr[i].prob;

        double half = total / 2.0;
        double accum = 0.0;
        int split = node.start;
        for (int i = node.start; i < node.end; i++) {
            accum += node.arr[i].prob;
            split = i + 1;
            if (accum >= half) break;
        }

        char left_p[256], right_p[256];
        memcpy(left_p, node.prefix, node.plen);
        memcpy(right_p, node.prefix, node.plen);
        left_p[node.plen] = '0';
        right_p[node.plen] = '1';

        stack[top++] = (SFNode){(SymbolProb*)symbols, node.start, split, left_p, node.plen + 1};
        stack[top++] = (SFNode){(SymbolProb*)symbols, split, node.end, right_p, node.plen + 1};
    }
}
