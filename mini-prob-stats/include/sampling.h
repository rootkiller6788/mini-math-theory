#ifndef SAMPLING_H
#define SAMPLING_H

/* L5: Simple Random Sampling */
void sample_random(int* population, int pop_size, int* sample, int sample_size);
void sample_random_replace(int* population, int pop_size, int* sample, int sample_size);

/* L5: Stratified Sampling */
void sample_stratified(int** strata, int* strata_sizes, int n_strata,
                       int* sample, int* sample_sizes);
double stratified_mean_estimate(double* means, double* weights, int n_strata);
double stratified_mean_variance(double* variances, double* weights,
                                 int* strata_sizes, int* sample_sizes,
                                 int n_strata);

/* L5: Systematic Sampling */
void sample_systematic(int* population, int pop_size, int* sample, int sample_size);

/* L7: Cluster Sampling (one-stage) */
void sample_cluster_one_stage(int** clusters, int* cluster_sizes, int n_clusters,
                               int n_select, int* sample, int* sample_size_out);

/* L5: Bootstrap */
void bootstrap_sample(double* data, int n, double* sample, int sample_size);
double bootstrap_mean_ci(double* data, int n, int n_bootstrap, double confidence,
                         double* lo, double* hi);

/* L5: Reservoir Sampling (streaming) */
int* reservoir_sample(int stream_len, int k);

/* L7: Design Effect */
double design_effect_simple(int cluster_size, double intraclass_corr);

/* L5: Sample size calculation */
double sample_size_proportion(double p, double margin, double confidence);
double sample_size_mean(double sigma, double margin, double confidence);

#endif
