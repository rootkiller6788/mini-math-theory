/**
 * Bloom Filter Implementation
 *
 * Uses double-hashing scheme to derive k hash functions from 2.
 * h_i(key) = (h1(key) + i * h2(key)) mod m
 *
 * Theorem: FPR ≈ (1 - e^{-kn/m})^k. Optimal k = (m/n) * ln(2).
 */

#include "bloom.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* FNV-1a hash (32-bit) */
static unsigned int fnv1a_hash(const char* key, unsigned int seed) {
    unsigned int hash = seed;
    while (*key) {
        hash ^= (unsigned char)*key++;
        hash *= 0x01000193;
    }
    return hash;
}

/* Jenkins OAAT hash */
static unsigned int jenkins_hash(const char* key, unsigned int seed) {
    unsigned int hash = seed;
    while (*key) {
        hash += (unsigned char)*key++;
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

BloomFilter* bloom_create(int expected_n, double false_positive_rate) {
    BloomFilter* bf = (BloomFilter*)malloc(sizeof(BloomFilter));
    
    /* m = -n * ln(p) / (ln 2)^2 */
    double m_dbl = -((double)expected_n * log(false_positive_rate)) / (log(2) * log(2));
    bf->m = (int)(m_dbl + 0.5);
    if (bf->m < 8) bf->m = 8;
    
    /* k = (m/n) * ln(2) */
    double k_dbl = ((double)bf->m / expected_n) * log(2);
    bf->k = (int)(k_dbl + 0.5);
    if (bf->k < 1) bf->k = 1;
    if (bf->k > 32) bf->k = 32;
    
    bf->n = expected_n;
    int bytes = (bf->m + 7) / 8;
    bf->bits = (unsigned char*)calloc((size_t)bytes, 1);
    
    return bf;
}

void bloom_free(BloomFilter* bf) {
    if (bf) {
        free(bf->bits);
        free(bf);
    }
}

static void bloom_get_hashes(const char* key, unsigned int* h1, unsigned int* h2) {
    *h1 = fnv1a_hash(key, 0x811C9DC5u);
    *h2 = jenkins_hash(key, 0x9E3779B9u);
}

void bloom_add(BloomFilter* bf, const char* key) {
    unsigned int h1, h2;
    bloom_get_hashes(key, &h1, &h2);

    for (int i = 0; i < bf->k; i++) {
        unsigned int bit = (h1 + (unsigned int)i * h2) % (unsigned int)bf->m;
        bf->bits[bit / 8] |= (unsigned char)(1u << (bit % 8));
    }
}

bool bloom_contains(BloomFilter* bf, const char* key) {
    unsigned int h1, h2;
    bloom_get_hashes(key, &h1, &h2);
    
    for (int i = 0; i < bf->k; i++) {
        unsigned int bit = (h1 + (unsigned int)i * h2) % (unsigned int)bf->m;
        if (!(bf->bits[bit / 8] & (1u << (bit % 8)))) {
            return false;
        }
    }
    return true;
}

int bloom_bit_size(BloomFilter* bf) { return bf->m; }
int bloom_hash_count(BloomFilter* bf) { return bf->k; }

double bloom_estimate_fpr(BloomFilter* bf) {
    /* p ≈ (1 - e^{-kn/m})^k */
    double exponent = -((double)bf->k * bf->n) / bf->m;
    double inner = 1.0 - exp(exponent);
    return pow(inner, bf->k);
}

void bloom_clear(BloomFilter* bf) {
    int bytes = (bf->m + 7) / 8;
    memset(bf->bits, 0, (size_t)bytes);
}
