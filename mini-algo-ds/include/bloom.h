/**
 * Bloom Filter - L7: Probabilistic Data Structure (Application)
 *
 * Space-efficient probabilistic set membership with false positive rate.
 * No false negatives. Uses k independent hash functions.
 *
 * Theorem: For n elements, m bits, k hash functions:
 *   False positive rate p ≈ (1 - e^{-kn/m})^k
 *   Optimal k = (m/n) * ln(2)
 *
 * Stanford CS 166: Data Structures / CMU 15-451: Algorithm Design
 */

#ifndef BLOOM_H
#define BLOOM_H

#include <stdbool.h>

typedef struct {
    unsigned char* bits;
    int m;           /* number of bits */
    int k;           /* number of hash functions */
    int n;           /* expected number of elements */
} BloomFilter;

BloomFilter* bloom_create(int expected_n, double false_positive_rate);
void bloom_free(BloomFilter* bf);
void bloom_add(BloomFilter* bf, const char* key);
bool bloom_contains(BloomFilter* bf, const char* key);
int  bloom_bit_size(BloomFilter* bf);
int  bloom_hash_count(BloomFilter* bf);
double bloom_estimate_fpr(BloomFilter* bf);
void bloom_clear(BloomFilter* bf);

#endif
