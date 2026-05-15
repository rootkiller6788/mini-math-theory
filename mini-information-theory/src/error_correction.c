#include "error_correction.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

void hamming74_encode(const bool data[4], bool encoded[7]) {
    encoded[2] = data[0];
    encoded[4] = data[1];
    encoded[5] = data[2];
    encoded[6] = data[3];
    encoded[0] = data[0] ^ data[1] ^ data[3];
    encoded[1] = data[0] ^ data[2] ^ data[3];
    encoded[3] = data[1] ^ data[2] ^ data[3];
}

int hamming74_decode(const bool received[7], bool decoded[4]) {
    bool s[3];
    hamming74_syndrome(received, s);
    int error_pos = s[0] + s[1] * 2 + s[2] * 4;

    bool corrected[7];
    for (int i = 0; i < 7; i++) corrected[i] = received[i];

    if (error_pos > 0 && error_pos <= 7) {
        corrected[error_pos - 1] = !corrected[error_pos - 1];
    } else if (error_pos > 7) {
        return -1;
    }

    decoded[0] = corrected[2];
    decoded[1] = corrected[4];
    decoded[2] = corrected[5];
    decoded[3] = corrected[6];
    return error_pos;
}

void hamming74_syndrome(const bool received[7], bool syndrome[3]) {
    syndrome[0] = received[0] ^ received[2] ^ received[4] ^ received[6];
    syndrome[1] = received[1] ^ received[2] ^ received[5] ^ received[6];
    syndrome[2] = received[3] ^ received[4] ^ received[5] ^ received[6];
}

void hamming84_encode(const bool data[4], bool encoded[8]) {
    hamming74_encode(data, encoded);
    bool parity = 0;
    for (int i = 0; i < 7; i++) parity ^= encoded[i];
    encoded[7] = parity;
}

int hamming84_decode(const bool received[8], bool decoded[4]) {
    bool syndrome[3];
    bool recv7[7];
    for (int i = 0; i < 7; i++) recv7[i] = received[i];

    hamming74_syndrome(recv7, syndrome);
    int err_pos = syndrome[0] + syndrome[1] * 2 + syndrome[2] * 4;

    bool parity = 0;
    for (int i = 0; i < 8; i++) parity ^= received[i];

    if (err_pos == 0 && parity == 0) {
        // No error
        decoded[0] = received[2];
        decoded[1] = received[4];
        decoded[2] = received[5];
        decoded[3] = received[6];
        return 0;
    } else if (err_pos > 0 && parity == 1) {
        // Single error, correctable
        bool corrected[7];
        for (int i = 0; i < 7; i++) corrected[i] = received[i];
        corrected[err_pos - 1] = !corrected[err_pos - 1];
        decoded[0] = corrected[2];
        decoded[1] = corrected[4];
        decoded[2] = corrected[5];
        decoded[3] = corrected[6];
        return 1;
    } else if (err_pos == 0 && parity == 1) {
        // Error in parity bit only
        decoded[0] = received[2];
        decoded[1] = received[4];
        decoded[2] = received[5];
        decoded[3] = received[6];
        return 1;
    } else {
        // Double error, detected but not corrected
        return -1;
    }
}

void add_bit_errors(const bool* input, int len, double error_prob, bool* output) {
    for (int i = 0; i < len; i++) {
        double r = (double)rand() / RAND_MAX;
        output[i] = (r < error_prob) ? !input[i] : input[i];
    }
}

void repetition_encode(bool bit, int n, bool* encoded) {
    for (int i = 0; i < n; i++)
        encoded[i] = bit;
}

bool repetition_decode(const bool* received, int n) {
    int ones = 0;
    for (int i = 0; i < n; i++)
        if (received[i]) ones++;
    return ones > n / 2;
}
