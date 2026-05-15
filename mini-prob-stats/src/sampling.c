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

void sample_random_replace(int* population, int pop_size, int* sample, int sample_size) {
    if (!population || !sample || pop_size <= 0 || sample_size <= 0) return;
    for (int i = 0; i < sample_size; i++) {
        sample[i] = population[rand() % pop_size];
    }
}

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
