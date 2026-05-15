#include "channel.h"
#include "entropy.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void bsc_init(BinarySymmetricChannel* ch, double error_prob) {
    ch->error_prob = error_prob;
    ch->input = NULL;
    ch->output = NULL;
    ch->length = 0;
    srand((unsigned)time(NULL));
}

double bsc_capacity(const BinarySymmetricChannel* ch) {
    return 1.0 - entropy_binary(ch->error_prob);
}

void bsc_transmit(BinarySymmetricChannel* ch, const bool* input, int len, bool* output) {
    for (int i = 0; i < len; i++) {
        double r = (double)rand() / RAND_MAX;
        output[i] = (r < ch->error_prob) ? !input[i] : input[i];
    }
}

double bsc_error_rate(const BinarySymmetricChannel* ch, const bool* original, int len) {
    int errors = 0;
    for (int i = 0; i < len; i++)
        if (ch->output[i] != original[i]) errors++;
    return (double)errors / len;
}

double bsc_mutual_information(double error_prob, double input_dist) {
    double p = input_dist;
    double pe = error_prob;
    double h_y = entropy_binary(p * (1 - pe) + (1 - p) * pe);
    double h_yx = entropy_binary(pe);
    return h_y - h_yx;
}

void channel_init(GeneralChannel* ch, int in_size, int out_size) {
    ch->input_size = in_size;
    ch->output_size = out_size;
    ch->matrix = malloc(in_size * sizeof(double*));
    for (int i = 0; i < in_size; i++) {
        ch->matrix[i] = calloc(out_size, sizeof(double));
    }
}

void channel_set_transition(GeneralChannel* ch, int input, int output, double prob) {
    if (input < ch->input_size && output < ch->output_size)
        ch->matrix[input][output] = prob;
}

void channel_free(GeneralChannel* ch) {
    for (int i = 0; i < ch->input_size; i++) free(ch->matrix[i]);
    free(ch->matrix);
}

double channel_capacity_blahut(GeneralChannel* ch, double epsilon, int max_iter) {
    int nx = ch->input_size, ny = ch->output_size;
    double* px = malloc(nx * sizeof(double));
    for (int i = 0; i < nx; i++) px[i] = 1.0 / nx;

    for (int iter = 0; iter < max_iter; iter++) {
        double* py = calloc(ny, sizeof(double));
        for (int j = 0; j < ny; j++)
            for (int i = 0; i < nx; i++)
                py[j] += px[i] * ch->matrix[i][j];

        double cap = 0.0;
        double* new_px = calloc(nx, sizeof(double));
        double norm = 0.0;

        for (int i = 0; i < nx; i++) {
            double sum = 0.0;
            for (int j = 0; j < ny; j++)
                if (ch->matrix[i][j] > 0 && py[j] > 0)
                    sum += ch->matrix[i][j] * log2(ch->matrix[i][j] / py[j]);
            new_px[i] = px[i] * pow(2.0, sum);
            norm += new_px[i];
        }

        for (int i = 0; i < nx; i++) new_px[i] /= norm;
        double old_cap = cap;

        double diff = 0.0;
        for (int i = 0; i < nx; i++) {
            diff += fabs(new_px[i] - px[i]);
            px[i] = new_px[i];
        }

        free(py); free(new_px);
        if (diff < epsilon) break;
    }

    double* py = calloc(ny, sizeof(double));
    for (int j = 0; j < ny; j++)
        for (int i = 0; i < nx; i++)
            py[j] += px[i] * ch->matrix[i][j];

    double cap = 0.0;
    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
            if (ch->matrix[i][j] > 0 && py[j] > 0)
                cap += px[i] * ch->matrix[i][j] * log2(ch->matrix[i][j] / py[j]);

    free(px); free(py);
    return cap;
}

double channel_capacity_uniform_input(GeneralChannel* ch) {
    int nx = ch->input_size, ny = ch->output_size;
    double* px = malloc(nx * sizeof(double));
    for (int i = 0; i < nx; i++) px[i] = 1.0 / nx;

    double* py = calloc(ny, sizeof(double));
    for (int j = 0; j < ny; j++)
        for (int i = 0; i < nx; i++)
            py[j] += px[i] * ch->matrix[i][j];

    double mi = 0.0;
    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
            if (ch->matrix[i][j] > 0 && py[j] > 0)
                mi += px[i] * ch->matrix[i][j] * log2(ch->matrix[i][j] / py[j]);

    free(px); free(py);
    return mi;
}
