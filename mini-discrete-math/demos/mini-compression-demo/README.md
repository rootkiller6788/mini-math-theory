# Mini Compression Demo — Huffman Coding for File Compression

## Overview

A file compression demonstration using Huffman coding — the same algorithm used in
gzip, PNG, JPEG, and MP3. The demo reads a file, builds a Huffman tree from byte
frequencies, generates prefix codes, and produces a compact binary encoding.

## Use Case

Huffman coding assigns shorter bit sequences to more frequent symbols and longer
sequences to rare ones, achieving optimal prefix-free compression for a given
frequency distribution. This is the entropy coding stage of virtually every modern
compression format.

## Architecture

```
Input File → Byte Frequency Count → Huffman Tree → Code Table → Bit Encoding → Compressed Output
                                                                                      ↓
Decompression:   Compressed Bits → Huffman Tree → Symbol Lookup → Original File
```

## Implementation Sketch

### Step 1: Read File and Count Byte Frequencies

```c
#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void count_byte_frequencies(const char* filename, int freq[256]) {
    memset(freq, 0, 256 * sizeof(int));

    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return;
    }

    int byte;
    while ((byte = fgetc(fp)) != EOF)
        freq[byte]++;

    fclose(fp);
    printf("File: %s\n", filename);
    printf("Unique bytes found: ");
    int unique = 0;
    for (int i = 0; i < 256; i++)
        if (freq[i] > 0) unique++;
    printf("%d\n\n", unique);
}
```

### Step 2: Build Huffman Tree and Generate Codes

```c
void build_huffman_codes(int freq[256], HuffmanNode** root, HuffmanCode codes[256]) {
    // Build the Huffman tree from frequency data
    // huffman_build_tree expects a text string; for byte-level compression,
    // we need to construct the nodes manually from freq array.

    // Alternative: use huffman_build_tree with a dummy string,
    // or construct nodes directly.

    int n = 0;
    HuffmanNode* nodes[256];

    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            HuffmanNode* node = malloc(sizeof(HuffmanNode));
            node->ch = (char)i;
            node->freq = freq[i];
            node->left = node->right = NULL;
            nodes[n++] = node;
        }
    }

    if (n == 0) { *root = NULL; return; }
    if (n == 1) {
        HuffmanNode* internal = malloc(sizeof(HuffmanNode));
        internal->ch = '\0';
        internal->freq = nodes[0]->freq;
        internal->left = nodes[0];
        internal->right = NULL;
        nodes[0] = internal;
    }

    int size = n;
    while (size > 1) {
        int min1 = 0, min2 = 1;
        if (nodes[min1]->freq > nodes[min2]->freq) {
            int t = min1; min1 = min2; min2 = t;
        }
        for (int i = 2; i < size; i++) {
            if (nodes[i]->freq < nodes[min1]->freq) {
                min2 = min1; min1 = i;
            } else if (nodes[i]->freq < nodes[min2]->freq) {
                min2 = i;
            }
        }

        HuffmanNode* internal = malloc(sizeof(HuffmanNode));
        internal->ch = '\0';
        internal->freq = nodes[min1]->freq + nodes[min2]->freq;
        internal->left = nodes[min1];
        internal->right = nodes[min2];

        nodes[min1] = internal;
        nodes[min2] = nodes[size - 1];
        size--;
    }

    *root = nodes[0];

    // Generate codes
    char code_buf[256];
    huffman_generate_codes(*root, code_buf, 0, codes);
}
```

### Step 3: Compute Compression Statistics

```c
typedef struct {
    long long original_size;      // bytes
    long long compressed_bits;    // bits
    long long unique_symbols;
    double entropy;               // theoretical minimum (bits per symbol)
    double avg_code_len;          // actual average bits per symbol
    double compression_ratio;
    double space_savings;
} CompressionStats;

CompressionStats compute_stats(const char* filename, int freq[256],
                                HuffmanCode codes[256]) {
    CompressionStats stats = {0};

    FILE* fp = fopen(filename, "rb");
    if (!fp) return stats;

    int byte;
    long long total_symbols = 0;
    while ((byte = fgetc(fp)) != EOF) {
        total_symbols++;
        stats.compressed_bits += codes[byte].code_len;
    }
    fclose(fp);

    stats.original_size = total_symbols;
    stats.unique_symbols = 0;
    double entropy_sum = 0.0;
    double weighted_code_len = 0.0;

    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            stats.unique_symbols++;
            double p = (double)freq[i] / total_symbols;
            entropy_sum -= p * log2(p);
            weighted_code_len += p * codes[i].code_len;
        }
    }

    stats.entropy = entropy_sum;
    stats.avg_code_len = weighted_code_len;
    stats.compression_ratio = (double)stats.compressed_bits /
                              (stats.original_size * 8) * 100.0;
    stats.space_savings = 100.0 - stats.compression_ratio;

    return stats;
}
```

### Step 4: Print a Compression Report

```c
void print_compression_report(const char* filename, CompressionStats* s) {
    printf("========================================\n");
    printf("  Huffman Compression Report: %s\n", filename);
    printf("========================================\n");
    printf("  Original size:    %lld bytes (%lld bits)\n",
           s->original_size, s->original_size * 8);
    printf("  Compressed size:  %lld bits (%.1f bytes)\n",
           s->compressed_bits, s->compressed_bits / 8.0);
    printf("  Unique symbols:   %lld\n", s->unique_symbols);
    printf("  ----------------------------------------\n");
    printf("  Shannon entropy:  %.4f bits/symbol\n", s->entropy);
    printf("  Avg code length:  %.4f bits/symbol\n", s->avg_code_len);
    printf("  Efficiency:       %.2f%% (vs theoretical optimum)\n",
           s->entropy / s->avg_code_len * 100.0);
    printf("  ----------------------------------------\n");
    printf("  Compression ratio: %.1f%% (of original)\n",
           s->compression_ratio);
    printf("  Space savings:     %.1f%%\n", s->space_savings);
    printf("========================================\n");
}
```

### Step 5: Encode and Decode the File

```c
typedef struct {
    unsigned char* data;
    long long bit_length;
} BitBuffer;

void encode_file(const char* filename, HuffmanCode codes[256],
                 BitBuffer* output) {
    // First pass: calculate total bits
    FILE* fp = fopen(filename, "rb");
    long long total_bits = 0;
    int byte;
    while ((byte = fgetc(fp)) != EOF)
        total_bits += codes[byte].code_len;
    fclose(fp);

    // Allocate buffer
    long long byte_count = (total_bits + 7) / 8;
    output->data = calloc(byte_count, 1);
    output->bit_length = total_bits;

    // Second pass: write bits
    fp = fopen(filename, "rb");
    long long bit_pos = 0;
    while ((byte = fgetc(fp)) != EOF) {
        HuffmanCode* hc = &codes[byte];
        for (int j = 0; j < hc->code_len; j++) {
            if (hc->code[j] == '1') {
                long long byte_idx = bit_pos / 8;
                int bit_idx = 7 - (bit_pos % 8);
                output->data[byte_idx] |= (1 << bit_idx);
            }
            bit_pos++;
        }
    }
    fclose(fp);
}

void decode_file(BitBuffer* input, HuffmanNode* root,
                 const char* output_filename) {
    FILE* fp = fopen(output_filename, "wb");
    if (!fp) return;

    HuffmanNode* curr = root;
    for (long long i = 0; i < input->bit_length; i++) {
        long long byte_idx = i / 8;
        int bit_idx = 7 - (i % 8);
        int bit = (input->data[byte_idx] >> bit_idx) & 1;

        curr = bit ? curr->right : curr->left;

        if (!curr->left && !curr->right) {
            fputc((unsigned char)curr->ch, fp);
            curr = root;
        }
    }

    fclose(fp);
}

void test_roundtrip(const char* filename, HuffmanCode codes[256],
                    HuffmanNode* root) {
    BitBuffer encoded;
    encode_file(filename, codes, &encoded);

    char outname[256];
    snprintf(outname, sizeof(outname), "%s.decoded", filename);
    decode_file(&encoded, root, outname);

    // Compare original with decoded
    FILE* f1 = fopen(filename, "rb");
    FILE* f2 = fopen(outname, "rb");
    int match = 1;
    int b1, b2;
    while ((b1 = fgetc(f1)) != EOF && (b2 = fgetc(f2)) != EOF) {
        if (b1 != b2) { match = 0; break; }
    }
    if (fgetc(f1) != EOF || fgetc(f2) != EOF) match = 0;
    fclose(f1); fclose(f2);

    printf("Roundtrip verification: %s\n", match ? "PASS ✓" : "FAIL ✗");

    free(encoded.data);
    remove(outname);
}
```

### Step 6: Display Code Table

```c
void print_code_table(int freq[256], HuffmanCode codes[256]) {
    printf("\n%-8s %-8s %-10s %s\n", "Byte", "Char", "Freq", "Code");
    printf("--------------------------------------\n");
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            printf("0x%02X   ", i);
            if (i >= 32 && i < 127)
                printf("'%c'     ", (char)i);
            else
                printf("' '     ");
            printf("%-10d ", freq[i]);
            for (int j = 0; j < codes[i].code_len; j++)
                printf("%c", codes[i].code[j]);
            printf(" (%d bits)\n", codes[i].code_len);
        }
    }
}
```

## Example: Compress a Text File

```c
void compress_file_demo(const char* filename) {
    int freq[256];
    count_byte_frequencies(filename, freq);

    HuffmanNode* root;
    HuffmanCode codes[256] = {0};
    build_huffman_codes(freq, &root, codes);

    print_code_table(freq, codes);

    CompressionStats stats = compute_stats(filename, freq, codes);
    print_compression_report(filename, &stats);

    test_roundtrip(filename, codes, root);

    huffman_free(root);
}

int main(void) {
    printf("====== Huffman File Compression Demo ======\n\n");

    // Compress this source file itself as a demo
    compress_file_demo("demos/mini-compression-demo/README.md");

    // Compare different file types
    // compress_file_demo("data/random.bin");     // high entropy, low compression
    // compress_file_demo("data/repeated.txt");   // low entropy, high compression
    // compress_file_demo("data/english.txt");    // ~4.7 bits/char Shannon entropy

    return 0;
}
```

## Understanding Compression Results

| File Type | Typical Entropy | Typical Compression | Why |
|-----------|----------------|---------------------|-----|
| English text | ~4.7 bits/char | 40-50% | Letter frequency distribution is skewed (e, t, a are common) |
| Source code | ~4.5 bits/char | 40-45% | Similar to English but with more punctuation |
| Random data | 8.0 bits/byte | 0% (may expand!) | Uniform distribution — no pattern to exploit |
| Repeated patterns | <1 bit/char | >85% | Very few unique symbols, extreme compression |
| Executables | ~6.5 bits/byte | 15-20% | More uniform distribution than text |

## Why Huffman is Not Applied to Entire Files Directly

In practice (gzip, PNG, etc.), Huffman coding is combined with:
1. **LZ77/LZSS**: Dictionary-based compression that replaces repeated strings with back-references
2. **Huffman on LZ output**: Huffman codes the (literal, length, distance) tokens from LZ
3. **Block-based encoding**: The input is split into blocks, each with its own Huffman tree

This demo applies Huffman directly to bytes for pedagogical clarity. Real compressors
use it as the final entropy coding stage after transform/ prediction steps.

## Building

```bash
cd mini-discrete-math
gcc -I include -o bin/compress \
    demos/mini-compression-demo/compress.c \
    src/tree.c -lm
```

## Dependencies

- `tree.h` / `tree.c` — `huffman_build_tree()`, `huffman_generate_codes()`,
  `huffman_encode()`, `huffman_decode()`, `huffman_free()`

## Further Exploration

- **Canonical Huffman Codes**: Store only the bit lengths per symbol instead of the full tree — used in DEFLATE (gzip) and JPEG.
- **Adaptive Huffman**: Update the tree as symbols arrive — no need to pre-scan the file (used in early modems, V.42bis).
- **Arithmetic Coding**: Achieves compression closer to entropy bound but is patent-encumbered (historically).
- **ANS (Asymmetric Numeral Systems)**: Modern replacement for both Huffman and arithmetic coding, used in Zstandard and recent codecs.
