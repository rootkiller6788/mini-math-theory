/* probability.c — Classical probability, combinatorics, limit theorems.
 *
 * L1: Probability axioms (Kolmogorov 1933)
 * L2: Conditional probability, Bayes theorem, independence
 * L4: Law of Large Numbers, Central Limit Theorem verification
 * L5: Poisson process, law of total probability
 * L8: Chernoff bound, Hoeffding inequality
 *
 * Theorem sources:
 *   Kolmogorov (1933): Foundations of Probability
 *   MIT 18.05 §1–§5
 *   Feller (1968): An Introduction to Probability Theory
 */

#include "probability.h"
#include <math.h>
#include <stdlib.h>

/* --------------------------------------------------------------------------
 * L1: Basic probability operations (Kolmogorov axioms)
 * -------------------------------------------------------------------------- */
double prob_union(double a, double b, double intersection) {
    if (a < 0.0) a = 0.0;
    if (a > 1.0) a = 1.0;
    if (b < 0.0) b = 0.0;
    if (b > 1.0) b = 1.0;
    return a + b - intersection;
}

double prob_intersection_independent(double a, double b) {
    return a * b;
}

double prob_conditional(double a_given_b_num, double b) {
    if (b == 0.0) return 0.0;
    return a_given_b_num / b;
}

double prob_bayes(double prior, double likelihood, double evidence) {
    if (evidence == 0.0) return 0.0;
    return (likelihood * prior) / evidence;
}

double prob_complement(double p) {
    return 1.0 - p;
}

/* --------------------------------------------------------------------------
 * L1: Combinatorics
 * -------------------------------------------------------------------------- */
int factorial(int n) {
    if (n < 0) return 0;
    if (n > 12) return 0;  /* 13! > INT_MAX */
    int result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

long long nCr(int n, int r) {
    if (r < 0 || n < 0 || r > n) return 0;
    if (r > n - r) r = n - r;
    long long result = 1;
    for (int i = 1; i <= r; i++) {
        result = result * (n - r + i) / i;
    }
    return result;
}

long long nPr(int n, int r) {
    if (r < 0 || n < 0 || r > n) return 0;
    long long result = 1;
    for (int i = 0; i < r; i++) {
        result *= (n - i);
    }
    return result;
}

/* Large factorial using double for approximation: Stirling's formula */
double factorial_approx(int n) {
    if (n < 0) return 0.0;
    if (n == 0) return 1.0;
    /* Stirling: n! ≈ √(2πn)·(n/e)^n */
    return sqrt(2.0 * 3.14159265358979323846 * n)
         * pow(n / 2.718281828459045, n);
}

/* Catalan numbers: C_n = C(2n, n) / (n+1) */
long long catalan(int n) {
    if (n < 0) return 0;
    return nCr(2 * n, n) / (n + 1);
}

/* Bell numbers B_n (first few: 1,1,2,5,15,52,203...) via recurrence */
long long bell_number(int n) {
    if (n < 0) return 0;
    if (n == 0) return 1;
    if (n > 10) return 0;  /* B_11 > 64-bit */
    long long bell[11];
    bell[0] = 1;
    for (int i = 1; i <= n; i++) {
        bell[i] = 0;
        for (int k = 0; k < i; k++) {
            bell[i] += nCr(i - 1, k) * bell[k];
        }
    }
    return bell[n];
}

/* Stirling numbers of the second kind S(n,k): partitions of n into k */
long long stirling_s2(int n, int k) {
    if (k == 0 && n == 0) return 1;
    if (k == 0 || n == 0) return 0;
    if (n > 12) return 0;
    return stirling_s2(n - 1, k - 1) + k * stirling_s2(n - 1, k);
}

/* --------------------------------------------------------------------------
 * L4: Law of Large Numbers (LLN) verification
 *
 * Weak LLN: X̄ₙ →_p μ
 * Strong LLN: X̄ₙ →_{a.s.} μ
 *
 * This simulates coin flips and tracks running proportion of heads.
 * -------------------------------------------------------------------------- */
double* llN_coin_flips(int n) {
    double* proportions = (double*)malloc(n * sizeof(double));
    if (!proportions) return NULL;
    int heads = 0;
    for (int i = 0; i < n; i++) {
        if (rand() % 2 == 0) heads++;
        proportions[i] = (double)heads / (i + 1);
    }
    return proportions;
}

/* LLN: running mean of Uniform[0,1] converging to 0.5 */
double* llN_uniform_mean(int n) {
    double* means = (double*)malloc(n * sizeof(double));
    if (!means) return NULL;
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += (double)rand() / RAND_MAX;
        means[i] = sum / (i + 1);
    }
    return means;
}

/* --------------------------------------------------------------------------
 * L4: Central Limit Theorem (CLT) verification
 *
 * CLT: √n·(X̄ₙ - μ)/σ →_d N(0,1) for i.i.d. with finite variance
 *
 * Generates m sample means from n i.i.d. Uniform(0,1), standardizes them,
 * then bins into a histogram for visualization.
 * -------------------------------------------------------------------------- */
void clt_histogram(int n_per_sample, int n_samples, double* bins, int n_bins,
                   int* counts) {
    if (n_per_sample <= 0 || n_samples <= 0 || n_bins <= 0) return;

    double mu = 0.5;           /* E[Uniform(0,1)] */
    double sigma = 1.0 / sqrt(12.0);  /* SD(Uniform(0,1)) */

    /* Clear counts */
    for (int b = 0; b < n_bins; b++) counts[b] = 0;

    double* standardized = (double*)malloc(n_samples * sizeof(double));
    if (!standardized) return;

    for (int s = 0; s < n_samples; s++) {
        double sum = 0.0;
        for (int i = 0; i < n_per_sample; i++) {
            sum += (double)rand() / RAND_MAX;
        }
        double xbar = sum / n_per_sample;
        /* CLT standardization: Z = √n (X̄ - μ) / σ */
        standardized[s] = sqrt((double)n_per_sample) * (xbar - mu) / sigma;
    }

    /* Find min/max for binning */
    double min_z = standardized[0], max_z = standardized[0];
    for (int i = 1; i < n_samples; i++) {
        if (standardized[i] < min_z) min_z = standardized[i];
        if (standardized[i] > max_z) max_z = standardized[i];
    }
    double range = max_z - min_z;
    if (range <= 0.0) range = 4.0;

    for (int s = 0; s < n_samples; s++) {
        int b = (int)((standardized[s] - min_z) / range * n_bins);
        if (b < 0) b = 0;
        if (b >= n_bins) b = n_bins - 1;
        counts[b]++;
    }

    for (int b = 0; b < n_bins; b++) {
        bins[b] = min_z + range * (b + 0.5) / n_bins;
    }

    free(standardized);
}

/* --------------------------------------------------------------------------
 * L5: Law of Total Probability
 * P(A) = Σ_i P(A|B_i) P(B_i)
 * where {B_i} partition the sample space
 * -------------------------------------------------------------------------- */
double law_of_total_probability(double* p_a_given_b, double* p_b, int n_parts) {
    if (n_parts <= 0 || !p_a_given_b || !p_b) return 0.0;
    double total = 0.0;
    for (int i = 0; i < n_parts; i++) {
        total += p_a_given_b[i] * p_b[i];
    }
    return total;
}

/* Extended Bayes: P(B_k|A) = P(A|B_k)P(B_k) / Σ_i P(A|B_i)P(B_i) */
double extended_bayes(double* p_a_given_b, double* p_b, int n_parts, int k) {
    if (k < 0 || k >= n_parts || n_parts <= 0) return 0.0;
    double evidence = law_of_total_probability(p_a_given_b, p_b, n_parts);
    if (evidence <= 0.0) return 0.0;
    return p_a_given_b[k] * p_b[k] / evidence;
}

/* --------------------------------------------------------------------------
 * L5: Poisson Process
 *
 * Inter-arrival times ~ Exponential(λ)
 * Number of arrivals in [0,T] ~ Poisson(λT)
 *
 * Simulates arrivals until time T, returns count and times.
 * Complexity: O(E[count]) = O(λT)
 * -------------------------------------------------------------------------- */
int poisson_process_simulate(double lambda, double T, double* arrival_times,
                              int max_arrivals) {
    if (lambda <= 0.0 || T <= 0.0 || max_arrivals <= 0) return 0;
    double t = 0.0;
    int count = 0;
    while (t < T && count < max_arrivals) {
        /* Generate exponential inter-arrival time ~ Exp(λ) */
        double u = (double)rand() / RAND_MAX;
        if (u <= 0.0) u = 1e-10;
        double inter = -log(u) / lambda;
        t += inter;
        if (t < T && count < max_arrivals) {
            if (arrival_times) arrival_times[count] = t;
            count++;
        }
    }
    return count;
}

/* Poisson process: probability of exactly k arrivals in [0,T] */
double poisson_process_prob_k(double lambda, double T, int k) {
    double rate = lambda * T;
    return pow(rate, k) * exp(-rate) / (double)factorial(k);
}

/* --------------------------------------------------------------------------
 * L8: Concentration inequalities
 *
 * Markov: P(X ≥ a) ≤ E[X]/a (for X ≥ 0)
 * Chebyshev: P(|X-μ| ≥ kσ) ≤ 1/k²
 * Chernoff (for sum of Bernoulli): P(S_n ≥ (1+δ)μ) ≤ exp(-δ²μ/3)
 * Hoeffding: P(|X̄-μ| ≥ t) ≤ 2 exp(-2nt²/(b-a)²)
 * -------------------------------------------------------------------------- */
double markov_bound(double expectation, double a) {
    if (a <= 0.0) return 1.0;
    double bound = expectation / a;
    return (bound < 1.0) ? bound : 1.0;
}

double chebyshev_bound(double variance, double k_sigma) {
    if (k_sigma <= 0.0) return 1.0;
    double bound = variance / (k_sigma * k_sigma);
    return (bound < 1.0) ? bound : 1.0;
}

double chernoff_bound_bernoulli(int n, double p, double delta) {
    /* For sum of Bernoulli: P(S/n ≥ (1+δ)p) ≤ exp(-δ²np/3) for δ∈(0,1) */
    if (delta <= 0.0) return 1.0;
    double bound = exp(-delta * delta * n * p / 3.0);
    return (bound < 1.0) ? bound : 1.0;
}

double hoeffding_bound(int n, double t, double a, double b) {
    /* P(|X̄-μ| ≥ t) ≤ 2 exp(-2nt²/(b-a)²) */
    if (t <= 0.0) return 1.0;
    double range = b - a;
    if (range <= 0.0) return 1.0;
    double bound = 2.0 * exp(-2.0 * n * t * t / (range * range));
    return (bound < 1.0) ? bound : 1.0;
}

/* --------------------------------------------------------------------------
 * L5: Random permutations — Fisher-Yates shuffle
 * Generates uniformly random permutation of {0,1,...,n-1}
 * Complexity: O(n)
 * -------------------------------------------------------------------------- */
void fisher_yates_shuffle(int* arr, int n) {
    if (!arr || n <= 0) return;
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

int* random_permutation(int n) {
    if (n <= 0) return NULL;
    int* perm = (int*)malloc(n * sizeof(int));
    if (!perm) return NULL;
    for (int i = 0; i < n; i++) perm[i] = i;
    fisher_yates_shuffle(perm, n);
    return perm;
}

/* --------------------------------------------------------------------------
 * L5: Derangements — permutations with no fixed points
 * D_n / n! → 1/e ≈ 0.3679 as n→∞
 * -------------------------------------------------------------------------- */
long long derangement(int n) {
    if (n < 0) return 0;
    if (n == 0) return 1;
    if (n == 1) return 0;
    if (n > 20) return 0;  /* D_21 > 64-bit */
    long long d0 = 1, d1 = 0, d2;
    for (int i = 2; i <= n; i++) {
        d2 = (i - 1) * (d1 + d0);
        d0 = d1;
        d1 = d2;
    }
    return d1;
}
