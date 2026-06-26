#ifndef ENTROPY_H
#define ENTROPY_H

#include <stdbool.h>

/* ── L1: Core Definitions ──────────────────────────────────────── */
typedef struct {
    double prob;
    char symbol;
} SymbolProb;

/** L2: Continuous distribution for differential entropy */
typedef struct {
    double (*pdf)(double x, const void* params);
    void* params;        /* distribution-specific parameters */
    double support_low;  /* lower support bound */
    double support_high; /* upper support bound */
} ContinuousDist;

/** L2: Discrete Markov Chain (finite state, stationary) */
typedef struct {
    double** transition; /* P[j][k] = prob of k given j */
    double* stationary;  /* stationary distribution π */
    int n_states;
} MarkovChain;

/* ── L1: Core Entropy API ──────────────────────────────────────── */
double entropy_shannon(const double* probs, int n);
double entropy_binary(double p);
double entropy_joint(const double** joint_probs, int nx, int ny);
double entropy_conditional(const double** joint_probs, int nx, int ny);
double entropy_of_text(const char* text);
double entropy_rate(const char* text, int block_size);
void   entropy_empirical_distribution(const char* text, SymbolProb* dist, int* n);
double entropy_from_counts(const int* counts, int n);

/* ── L2: Chain Rule ────────────────────────────────────────────── */
/** L2 Theorem: H(X,Y) = H(X) + H(Y|X). Verifies by computing difference. */
double entropy_chain_rule_verify(const double** joint_probs, int nx, int ny);

/* ── L3: Differential Entropy ──────────────────────────────────── */
/** L3: h(X) = -∫ p(x) log p(x) dx (numerical quadrature approximation)
 *  For Gaussian: h(X) = 0.5 log2(2πe σ²) */
double entropy_differential(const ContinuousDist* dist, int n_intervals);

/** L3: Gaussian differential entropy, exact analytic formula */
double entropy_differential_gaussian(double variance);

/* ── L4: Asymptotic Equipartition Property (AEP) ───────────────── */
/** L4: Section 3.1 of Cover & Thomas.
 *  Checks if sequence's empirical entropy is within ε of true entropy.
 *  Returns true if |(-1/n)log P(x^n) - H| ≤ ε */
bool aep_is_typical(const int* sequence, int n, const double* probs, int alphabet,
                    double epsilon);

/** L4: Size of typical set ≈ 2^{n(H+ε)} */
double aep_typical_set_size_estimate(int n, double entropy, double epsilon);

/** L4: Compute (-1/n) log2 P(x^n) — the empirical entropy of a sequence */
double aep_empirical_entropy(const int* sequence, int n, const double* probs, int alphabet);

/* ── L5: Entropy Rate of Markov Chain ──────────────────────────── */
/** L5: H(X) = -Σ_i π_i Σ_j P_{ij} log P_{ij} */
double entropy_rate_markov(const MarkovChain* mc);

/** L5: compute stationary distribution of Markov chain (power method) */
bool markov_stationary_distribution(MarkovChain* mc, double tol, int max_iter);

void markov_chain_free(MarkovChain* mc);

/* ── L4: Fano's Inequality ────────────────────────────────────── */
/** L4: P_e ≥ (H(X|Y) - 1) / log2(|X|). Returns lower bound on error prob. */
double fano_inequality_lower_bound(double h_cond, int alphabet_size);

/** L4: Verify Fano's inequality with actual joint distribution. */
double fano_inequality_verify(const double** joint_probs, int nx, int ny);

/* ── L2: Entropy Properties ────────────────────────────────────── */
/** L2: Maximum entropy for discrete alphabet of size n = log2(n) */
double entropy_max_uniform(int alphabet_size);

/** L2: Verify concavity: H(λp + (1-λ)q) ≥ λH(p) + (1-λ)H(q) */
double entropy_concavity_verify(const double* p, const double* q, int n, double lambda);

/** L2: Grouping axiom: verify H(p₁,...,pₖ) = H(p₁+...+pₖ) + ... */
double entropy_grouping_verify(const double* probs, int n, int split_point);

/** L5: Relative entropy D(p||u) from uniform, i.e., log n - H(p) */
double entropy_gap_from_uniform(const double* probs, int n);

#endif
