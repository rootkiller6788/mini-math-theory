#ifndef SAMPLING_H
#define SAMPLING_H

void sample_random(int* population, int pop_size, int* sample, int sample_size);
void sample_random_replace(int* population, int pop_size, int* sample, int sample_size);

void sample_stratified(int** strata, int* strata_sizes, int n_strata,
                       int* sample, int* sample_sizes);

void bootstrap_sample(double* data, int n, double* sample, int sample_size);
double bootstrap_mean_ci(double* data, int n, int n_bootstrap, double confidence,
                         double* lo, double* hi);

int* reservoir_sample(int stream_len, int k);

#endif
