#include "entropy.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

static double safe_log2(double x) {
    if (x <= 0.0) return 0.0;
    return log2(x);
}

double entropy_shannon(const double* probs, int n) {
    double h = 0.0;
    for (int i = 0; i < n; i++)
        if (probs[i] > 0.0)
            h -= probs[i] * safe_log2(probs[i]);
    return h;
}

double entropy_binary(double p) {
    if (p <= 0.0 || p >= 1.0) return 0.0;
    return -p * log2(p) - (1.0 - p) * log2(1.0 - p);
}

double entropy_joint(const double** joint_probs, int nx, int ny) {
    double h = 0.0;
    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
            if (joint_probs[i][j] > 0.0)
                h -= joint_probs[i][j] * safe_log2(joint_probs[i][j]);
    return h;
}

double entropy_conditional(const double** joint_probs, int nx, int ny) {
    double* py = calloc(ny, sizeof(double));
    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
            py[j] += joint_probs[i][j];

    double h_cond = 0.0;
    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
            if (joint_probs[i][j] > 0.0)
                h_cond -= joint_probs[i][j] * safe_log2(joint_probs[i][j] / py[j]);

    free(py);
    return h_cond;
}

double entropy_of_text(const char* text) {
    int counts[256] = {0};
    int total = 0;
    for (int i = 0; text[i]; i++) {
        counts[(unsigned char)text[i]]++;
        total++;
    }
    if (total == 0) return 0.0;

    double h = 0.0;
    for (int i = 0; i < 256; i++) {
        if (counts[i] > 0) {
            double p = (double)counts[i] / total;
            h -= p * log2(p);
        }
    }
    return h;
}

double entropy_rate(const char* text, int block_size) {
    int len = strlen(text);
    int nblocks = (len / block_size) * block_size;
    if (nblocks == 0) return 0.0;

    double total_h = 0.0;
    for (int i = 0; i < nblocks; i += block_size) {
        char block[32] = {0};
        memcpy(block, text + i, block_size < 32 ? block_size : 31);
        total_h += entropy_of_text(block);
    }
    return total_h / (nblocks / block_size) / block_size;
}

void entropy_empirical_distribution(const char* text, SymbolProb* dist, int* n) {
    int counts[256] = {0};
    int total = 0;
    for (int i = 0; text[i]; i++) {
        counts[(unsigned char)text[i]]++;
        total++;
    }
    *n = 0;
    for (int i = 0; i < 256; i++) {
        if (counts[i] > 0) {
            dist[*n].prob = (double)counts[i] / total;
            dist[*n].symbol = (char)i;
            (*n)++;
        }
    }
}

double entropy_from_counts(const int* counts, int n) {
    int total = 0;
    for (int i = 0; i < n; i++) total += counts[i];
    if (total == 0) return 0.0;

    double h = 0.0;
    for (int i = 0; i < n; i++) {
        if (counts[i] > 0) {
            double p = (double)counts[i] / total;
            h -= p * log2(p);
        }
    }
    return h;
}
