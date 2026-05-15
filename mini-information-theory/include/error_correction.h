#ifndef ERROR_CORRECTION_H
#define ERROR_CORRECTION_H

#include <stdbool.h>

// Hamming(7,4): 4 data bits → 7 encoded bits
// Can detect and correct 1-bit errors
void hamming74_encode(const bool data[4], bool encoded[7]);

// Returns true if no error detected, false if 1-bit error corrected
// Returns -1 via *error_pos if uncorrectable
int hamming74_decode(const bool received[7], bool decoded[4]);

// Syndrome: 3 bits identifying error position (0 = no error, 1-7 = bit position)
void hamming74_syndrome(const bool received[7], bool syndrome[3]);

// Hamming(8,4): adds overall parity bit for SEC-DED
void hamming84_encode(const bool data[4], bool encoded[8]);
int hamming84_decode(const bool received[8], bool decoded[4]);

// Bit error rate simulator
void add_bit_errors(const bool* input, int len, double error_prob, bool* output);

// Repetition code: (n,1) — simplest code, n copies
void repetition_encode(bool bit, int n, bool* encoded);
bool repetition_decode(const bool* received, int n);

#endif
