#ifndef CHANNEL_H
#define CHANNEL_H

#include <stdbool.h>

typedef struct {
    double error_prob;       // ε: bit flip probability
    bool* input;
    bool* output;
    int length;
} BinarySymmetricChannel;

void bsc_init(BinarySymmetricChannel* ch, double error_prob);
double bsc_capacity(const BinarySymmetricChannel* ch);
void bsc_transmit(BinarySymmetricChannel* ch, const bool* input, int len, bool* output);
double bsc_error_rate(const BinarySymmetricChannel* ch, const bool* original, int len);
double bsc_mutual_information(double error_prob, double input_dist);

typedef struct {
    double** matrix;   // transition matrix: P(output|input)
    int input_size;
    int output_size;
} GeneralChannel;

void channel_init(GeneralChannel* ch, int in_size, int out_size);
void channel_set_transition(GeneralChannel* ch, int input, int output, double prob);
double channel_capacity_blahut(GeneralChannel* ch, double epsilon, int max_iter);
void channel_free(GeneralChannel* ch);

double channel_capacity_uniform_input(GeneralChannel* ch);

#endif
