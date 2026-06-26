/* sampling.c — Survey sampling: random, stratified, systematic, cluster, bootstrap.
 *
 * L5: Simple Random Sampling (without / with replacement)
 * L5: Stratified Sampling, Systematic Sampling
 * L5: Bootstrap, Jackknife, Reservoir Sampling
 * L7: Cluster Sampling, Design Effect
 *
 * Theorem sources:
 *   Cochran (1977): Sampling Techniques
 *   Efron (1979): Bootstrap Methods
 *   Vitter (1985): Reservoir Sampling
 */

#include "sampling.h"
#include "inference.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int cmp_double_asc(const void* a, const void* b) {
    double da = *(const double*)a;
    double db = *(const double*)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

/* --------------------------------------------------------------------------
 * L5: Simple Random Sampling Without Replacement (SRSWOR)
 *
 * Fisher-Yates partial shuffle: select k from population of size N.
 * Every subset of size k has equal probability 1/C(N,k).
 * Complexity: O(N) time, O(N) space
 * -------------------------------------------------------------------------- */
void sample_random(int* population, int pop_size, int* sample, int sample_size) {
    if (!population || !sample || sample_size > pop_size || sample_size <= 0) return;
    int* temp = (int*)malloc(pop_size * sizeof(int));
    if (!temp) return;
    memcpy(temp, population, pop_size * sizeof(int));

    for (int i = 0; i < sample_size; i++) {
        int j = i + rand() % (pop_size - i);
        int tmp = temp[i];
        temp[i] = temp[j];
        temp[j] = tmp;
        sample[i] = temp[i];
    }
    free(temp);
}

/* --------------------------------------------------------------------------
 * L5: Simple Random Sampling With Replacement (SRSWR)
 *
 * Each draw is independent; population unchanged.
 * Bootstrap resampling is SRSWR with sample_size = pop_size.
 * -------------------------------------------------------------------------- */
void sample_random_replace(int* population, int pop_size, int* sample, int sample_size) {
    if (!population || !sample || pop_size <= 0 || sample_size <= 0) return;
    for (int i = 0; i < sample_size; i++) {
        sample[i] = population[rand() % pop_size];
    }
}

/* --------------------------------------------------------------------------
 * L5: Stratified Sampling
 *
 * Partition population into strata, SRS within each stratum.
 * Proportional allocation: n_h ∝ N_h
 * Neyman allocation: n_h ∝ N_h·σ_h (optimal for fixed total sample size)
 *
 * This implements proportional allocation by default.
 * -------------------------------------------------------------------------- */
void sample_stratified(int** strata, int* strata_sizes, int n_strata,
                       int* sample, int* sample_sizes) {
    if (!strata || !strata_sizes || !sample || !sample_sizes) return;
    int offset = 0;
    for (int s = 0; s < n_strata; s++) {
        int sz = strata_sizes[s];
        int ss = sample_sizes[s];
        if (sz > 0 && ss > 0) {
            sample_random(strata[s], sz, sample + offset, ss);
            offset += ss;
        }
    }
}

/* --------------------------------------------------------------------------
 * L5: Systematic Sampling
 *
 * Select every K-th element starting from random position.
 * K = pop_size / sample_size (sampling interval).
 *
 * Efficient, good coverage, but sensitive to periodicity.
 * Complexity: O(sample_size)
 * -------------------------------------------------------------------------- */
void sample_systematic(int* population, int pop_size, int* sample, int sample_size) {
    if (!population || !sample || pop_size <= 0 || sample_size <= 0) return;
    if (sample_size > pop_size) sample_size = pop_size;

    int k = pop_size / sample_size;
    if (k <= 0) k = 1;
    int start = rand() % k;

    for (int i = 0; i < sample_size; i++) {
        int idx = start + i * k;
        if (idx >= pop_size) idx = pop_size - 1;
        sample[i] = population[idx];
    }
}

/* --------------------------------------------------------------------------
 * L7: Cluster Sampling
 *
 * Population divided into clusters (e.g., geographic regions).
 * Randomly select clusters, then sample all (one-stage) or some (two-stage)
 * elements within selected clusters.
 *
 * This implements one-stage cluster sampling.
 * Complexity: O(num_clusters + total_sample)
 * -------------------------------------------------------------------------- */
void sample_cluster_one_stage(int** clusters, int* cluster_sizes, int n_clusters,
                               int n_select, int* sample, int* sample_size_out) {
    if (!clusters || !cluster_sizes || !sample || !sample_size_out) return;
    if (n_select > n_clusters) n_select = n_clusters;

    /* Randomly select clusters without replacement */
    int* cluster_idx = (int*)malloc(n_clusters * sizeof(int));
    if (!cluster_idx) return;
    for (int i = 0; i < n_clusters; i++) cluster_idx[i] = i;

    /* Fisher-Yates partial for clusters */
    for (int i = 0; i < n_select; i++) {
        int j = i + rand() % (n_clusters - i);
        int tmp = cluster_idx[i];
        cluster_idx[i] = cluster_idx[j];
        cluster_idx[j] = tmp;
    }

    /* Copy selected clusters' data */
    int offset = 0;
    for (int s = 0; s < n_select; s++) {
        int c = cluster_idx[s];
        memcpy(sample + offset, clusters[c], cluster_sizes[c] * sizeof(int));
        offset += cluster_sizes[c];
    }
    *sample_size_out = offset;
    free(cluster_idx);
}

/* --------------------------------------------------------------------------
 * L5: Bootstrap Resampling
 *
 * Draw n samples with replacement from observed data.
 * Non-parametric estimate of sampling distribution.
 * -------------------------------------------------------------------------- */
void bootstrap_sample(double* data, int n, double* sample, int sample_size) {
    if (!data || !sample || n <= 0 || sample_size <= 0) return;
    for (int i = 0; i < sample_size; i++) {
        sample[i] = data[rand() % n];
    }
}

double bootstrap_mean_ci(double* data, int n, int n_bootstrap, double confidence,
                         double* lo, double* hi) {
    if (n <= 0 || n_bootstrap <= 0 || !data) {
        *lo = *hi = 0.0;
        return 0.0;
    }

    double* boot_means = (double*)malloc(n_bootstrap * sizeof(double));
    if (!boot_means) { *lo = *hi = 0.0; return 0.0; }

    double* boot_samp = (double*)malloc(n * sizeof(double));
    if (!boot_samp) { free(boot_means); *lo = *hi = 0.0; return 0.0; }

    for (int b = 0; b < n_bootstrap; b++) {
        bootstrap_sample(data, n, boot_samp, n);
        boot_means[b] = mean(boot_samp, n);
    }

    qsort(boot_means, n_bootstrap, sizeof(double), cmp_double_asc);

    double alpha = (1.0 - confidence) / 2.0;
    int idx_lo = (int)(alpha * n_bootstrap);
    int idx_hi = (int)((1.0 - alpha) * n_bootstrap);
    if (idx_lo < 0) idx_lo = 0;
    if (idx_hi >= n_bootstrap) idx_hi = n_bootstrap - 1;

    *lo = boot_means[idx_lo];
    *hi = boot_means[idx_hi];

    double orig_mean = mean(data, n);
    free(boot_means);
    free(boot_samp);
    return orig_mean;
}

/* --------------------------------------------------------------------------
 * L5: Reservoir Sampling (Algorithm R, Vitter 1985)
 *
 * Uniformly sample k items from a stream of unknown length N.
 * After processing stream, each of the N items has equal probability k/N.
 *
 * Complexity: O(N) time, O(k) space
 * Key insight: for i ≥ k, keep item i with probability k/i,
 *              replacing a randomly chosen existing item.
 * -------------------------------------------------------------------------- */
int* reservoir_sample(int stream_len, int k) {
    if (k <= 0 || stream_len <= 0) return NULL;
    if (k > stream_len) k = stream_len;

    int* reservoir = (int*)malloc(k * sizeof(int));
    if (!reservoir) return NULL;

    for (int i = 0; i < k; i++) {
        reservoir[i] = i;
    }

    for (int i = k; i < stream_len; i++) {
        int j = rand() % (i + 1);
        if (j < k) {
            reservoir[j] = i;
        }
    }

    return reservoir;
}

/* --------------------------------------------------------------------------
 * L7: Design Effect (Deff)
 *
 * Deff = Var(complex_design) / Var(srs)
 * Measure of efficiency loss due to complex sampling design.
 *
 * For cluster sampling with equal-sized clusters of size m
 * and intraclass correlation ρ: Deff ≈ 1 + (m-1)ρ
 *
 * Deff > 1: less efficient than SRS
 * Deff < 1: more efficient (stratification can achieve this)
 * -------------------------------------------------------------------------- */
double design_effect_simple(int cluster_size, double intraclass_corr) {
    return 1.0 + (cluster_size - 1.0) * intraclass_corr;
}

/* --------------------------------------------------------------------------
 * L5: Stratified mean estimator with known stratum sizes
 *
 * ȳ_{st} = Σ W_h · ȳ_h
 * where W_h = N_h / N (stratum weight)
 *
 * Var(ȳ_{st}) = Σ W_h² · s_h²/n_h · (1 - n_h/N_h)  [with finite pop correction]
 * -------------------------------------------------------------------------- */
double stratified_mean_estimate(double* means, double* weights, int n_strata) {
    if (n_strata <= 0 || !means || !weights) return 0.0;
    double est = 0.0;
    for (int h = 0; h < n_strata; h++) {
        est += weights[h] * means[h];
    }
    return est;
}

double stratified_mean_variance(double* variances, double* weights,
                                 int* strata_sizes, int* sample_sizes,
                                 int n_strata) {
    if (n_strata <= 0 || !variances || !weights) return 0.0;
    double var = 0.0;
    for (int h = 0; h < n_strata; h++) {
        double wh_sq = weights[h] * weights[h];
        double fpc = 1.0;
        if (strata_sizes && sample_sizes &&
            strata_sizes[h] > 0 && sample_sizes[h] < strata_sizes[h]) {
            fpc = 1.0 - (double)sample_sizes[h] / strata_sizes[h];
        }
        var += wh_sq * variances[h] / sample_sizes[h] * fpc;
    }
    return var;
}

/* --------------------------------------------------------------------------
 * L5: Sample size calculation for desired margin of error
 *
 * For proportion: n = z²_{α/2} p(1-p) / e²
 * For mean:       n = z²_{α/2} σ² / e²
 *
 * Without finite population correction (conservative).
 * -------------------------------------------------------------------------- */
double sample_size_proportion(double p, double margin, double confidence) {
    if (margin <= 0.0) return 0.0;
    /* For 95% CI, z = 1.96 */
    double z = 1.96;
    if (confidence > 0.99) z = 2.576;
    if (confidence <= 0.90) z = 1.645;

    /* Conservative: use p=0.5 if not specified */
    if (p < 0.0 || p > 1.0) p = 0.5;
    return ceil(z * z * p * (1.0 - p) / (margin * margin));
}

double sample_size_mean(double sigma, double margin, double confidence) {
    if (margin <= 0.0 || sigma <= 0.0) return 0.0;
    double z = 1.96;
    if (confidence > 0.99) z = 2.576;
    if (confidence <= 0.90) z = 1.645;
    return ceil(z * z * sigma * sigma / (margin * margin));
}
