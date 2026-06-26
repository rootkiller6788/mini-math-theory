/* entropy.c — Information Theory: Entropy, AEP, Differential Entropy
 * Reference: Cover & Thomas, Elements of Information Theory, 2nd Ed.
 * MIT 6.441: Weeks 1-2
 *
 * Key formulas:
 *   H(X) = -Σ p(x) log₂ p(x)          [Definition 2.1]
 *   H(Y|X) = -Σ p(x,y) log₂ p(y|x)    [Definition 2.3]
 *   H(X,Y) = H(X) + H(Y|X)            [Theorem 2.2.1: Chain Rule]
 *   h(X) = -∫ f(x) log₂ f(x) dx       [Differential entropy, Ch.8]
 *   AEP: -1/n log P(xⁿ) → H(X)        [Theorem 3.1.1]
 *   |Aεⁿ| ≈ 2^{nH}                     [Theorem 3.1.2]
 *   Fano: P_e ≥ (H(X|Y)-1)/log|X|      [Theorem 2.10.1]
 */
#include "entropy.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E  2.71828182845904523536
#endif

/* ── Helper ─────────────────────────────────────────────────────── */
static double safe_log2(double x) {
    if (x <= 0.0) return 0.0;
    return log2(x);
}

static double safe_log(double x) {
    if (x <= 0.0) return 0.0;
    return log(x);
}

/* ── L1: Shannon Entropy ────────────────────────────────────────── */
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
    if (!py) return 0.0;
    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
            py[j] += joint_probs[i][j];

    double h_cond = 0.0;
    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
            if (joint_probs[i][j] > 0.0 && py[j] > 0.0)
                h_cond -= joint_probs[i][j] * log2(joint_probs[i][j] / py[j]);

    free(py);
    return h_cond;
}

double entropy_of_text(const char* text) {
    if (!text) return 0.0;
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
    if (!text || block_size < 1) return 0.0;
    int len = strlen(text);
    int nblocks = (len / block_size) * block_size;
    if (nblocks == 0) return 0.0;

    double total_h = 0.0;
    int actual_blocks = 0;
    for (int i = 0; i < nblocks; i += block_size) {
        char* block = malloc(block_size + 1);
        if (!block) continue;
        int copy_len = (i + block_size <= len) ? block_size : len - i;
        memcpy(block, text + i, copy_len);
        block[copy_len] = '\0';
        total_h += entropy_of_text(block);
        actual_blocks++;
        free(block);
    }
    return actual_blocks > 0 ? total_h / actual_blocks : 0.0;
}

void entropy_empirical_distribution(const char* text, SymbolProb* dist, int* n) {
    if (!text || !dist || !n) return;
    int counts[256] = {0};
    int total = 0;
    for (int i = 0; text[i]; i++) {
        counts[(unsigned char)text[i]]++;
        total++;
    }
    *n = 0;
    if (total == 0) return;
    for (int i = 0; i < 256; i++) {
        if (counts[i] > 0) {
            dist[*n].prob = (double)counts[i] / total;
            dist[*n].symbol = (char)i;
            (*n)++;
        }
    }
}

double entropy_from_counts(const int* counts, int n) {
    if (!counts || n <= 0) return 0.0;
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

/* ── L2: Chain Rule ─────────────────────────────────────────────── */
double entropy_chain_rule_verify(const double** joint_probs, int nx, int ny) {
    /* H(X,Y) = H(Y) + H(X|Y) = H(X) + H(Y|X)
     * entropy_conditional(joint_probs, nx, ny) returns H(X|Y).
     * So we verify: H(X,Y) = H(Y) + H(X|Y). */
    double h_xy = entropy_joint(joint_probs, nx, ny);

    double* py = calloc(ny, sizeof(double));
    if (!py) return -1.0;
    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
            py[j] += joint_probs[i][j];

    double hy = entropy_shannon(py, ny);
    double hx_given_y = entropy_conditional(joint_probs, nx, ny);
    free(py);

    return fabs(h_xy - (hy + hx_given_y));
}

/* ── L3: Differential Entropy ───────────────────────────────────── */
double entropy_differential(const ContinuousDist* dist, int n_intervals) {
    if (!dist || !dist->pdf || n_intervals < 2) return 0.0;

    double a = dist->support_low;
    double b = dist->support_high;
    if (a >= b) return 0.0;

    double dx = (b - a) / n_intervals;
    double h = 0.0;
    int valid_points = 0;

    /* Composite Simpson's rule for ∫ -p(x) log₂ p(x) dx */
    for (int i = 0; i <= n_intervals; i++) {
        double x = a + i * dx;
        if (x < a) x = a;
        if (x > b) x = b;

        double fx = dist->pdf(x, dist->params);
        if (fx <= 0.0) continue;

        double weight;
        if (i == 0 || i == n_intervals) weight = 1.0;
        else if (i % 2 == 1) weight = 4.0;
        else weight = 2.0;

        h -= weight * fx * log2(fx);
        valid_points++;
    }

    if (valid_points == 0) return 0.0;
    return h * dx / 3.0;
}

double entropy_differential_gaussian(double variance) {
    /* h(X) = 0.5 log₂(2πe σ²) — Cover & Thomas, Theorem 8.4.1 */
    if (variance <= 0.0) return 0.0;
    return 0.5 * log2(2.0 * M_PI * M_E * variance);
}

/* ── L4: Asymptotic Equipartition Property (AEP) ────────────────── */
double aep_empirical_entropy(const int* sequence, int n,
                              const double* probs, int alphabet) {
    if (!sequence || !probs || n <= 0 || alphabet <= 0) return 0.0;

    double sum_log_p = 0.0;
    for (int i = 0; i < n; i++) {
        int sym = sequence[i];
        if (sym < 0 || sym >= alphabet) continue;
        if (probs[sym] <= 0.0) return 1e10; /* impossible under model → infinite */
        sum_log_p += log2(probs[sym]);
    }
    return -sum_log_p / n;
}

bool aep_is_typical(const int* sequence, int n, const double* probs,
                    int alphabet, double epsilon) {
    if (!sequence || !probs || n <= 0 || alphabet <= 0) return false;

    double emp_h = aep_empirical_entropy(sequence, n, probs, alphabet);
    double true_h = entropy_shannon(probs, alphabet);

    return fabs(emp_h - true_h) <= epsilon;
}

double aep_typical_set_size_estimate(int n, double entropy, double epsilon) {
    /* |Aεⁿ| ≈ 2^{n(H+ε)}  —  Cover & Thomas, Theorem 3.1.2 */
    if (n <= 0) return 0.0;
    return pow(2.0, n * (entropy + epsilon));
}

/* ── L5: Entropy Rate of Markov Chain ───────────────────────────── */
bool markov_stationary_distribution(MarkovChain* mc, double tol, int max_iter) {
    if (!mc || mc->n_states <= 0 || !mc->transition) return false;

    int n = mc->n_states;
    double* pi = malloc(n * sizeof(double));
    double* pi_new = malloc(n * sizeof(double));
    if (!pi || !pi_new) { free(pi); free(pi_new); return false; }

    /* initialize as uniform */
    for (int i = 0; i < n; i++) pi[i] = 1.0 / n;

    for (int iter = 0; iter < max_iter; iter++) {
        for (int j = 0; j < n; j++) {
            pi_new[j] = 0.0;
            for (int k = 0; k < n; k++)
                pi_new[j] += pi[k] * mc->transition[k][j];
        }

        double diff = 0.0;
        for (int k = 0; k < n; k++) {
            diff += fabs(pi_new[k] - pi[k]);
            pi[k] = pi_new[k];
        }
        if (diff < tol) break;
    }

    mc->stationary = pi_new;
    free(pi);
    return true;
}

double entropy_rate_markov(const MarkovChain* mc) {
    /* H(X) = -Σ_i π_i Σ_j P_{ij} log₂ P_{ij}   [Cover & Thomas, Sec 4.2] */
    if (!mc || !mc->stationary || !mc->transition || mc->n_states <= 0)
        return 0.0;

    int n = mc->n_states;
    double h = 0.0;
    for (int i = 0; i < n; i++) {
        if (mc->stationary[i] <= 0.0) continue;
        for (int j = 0; j < n; j++) {
            if (mc->transition[i][j] > 0.0)
                h -= mc->stationary[i] * mc->transition[i][j] * log2(mc->transition[i][j]);
        }
    }
    return h;
}

void markov_chain_free(MarkovChain* mc) {
    if (!mc) return;
    if (mc->transition) {
        for (int i = 0; i < mc->n_states; i++) free(mc->transition[i]);
        free(mc->transition);
    }
    free(mc->stationary);
    free(mc);
}

/* ── L4: Fano's Inequality ──────────────────────────────────────── */
double fano_inequality_lower_bound(double h_cond, int alphabet_size) {
    /* Cover & Thomas, Theorem 2.10.1:
     * H(P_e) + P_e log(|X|-1) ≥ H(X|Y)
     * Approximated bound: P_e ≥ (H(X|Y) - 1) / log₂(|X|)
     * We return the tighter bound by solving H_b(P_e) + P_e log(|X|-1) = H(X|Y)
     * using binary search on P_e ∈ [0, 1-1/|X|]. */
    if (alphabet_size <= 1) return 0.0;
    if (h_cond <= 0.0) return 0.0;

    double log_card = log2((double)(alphabet_size - 1));
    double low = 0.0, high = 1.0 - 1.0 / alphabet_size;

    for (int iter = 0; iter < 50; iter++) {
        double mid = (low + high) / 2.0;
        double h_pe = entropy_binary(mid);
        double rhs = h_pe + mid * log_card;
        if (rhs < h_cond) low = mid;
        else high = mid;
    }
    return (low + high) / 2.0;
}

double fano_inequality_verify(const double** joint_probs, int nx, int ny) {
    /* Compute H(X|Y) and estimate P_e from the optimal decoder.
     * Returns: max(0, lower_bound - actual_conditional_entropy approach).
     * This returns the gap between H(X|Y)/log|X| and the Fano bound. */
    if (!joint_probs || nx <= 0 || ny <= 0) return -1.0;

    double h_cond = entropy_conditional(joint_probs, nx, ny);
    double lower = fano_inequality_lower_bound(h_cond, nx);

    /* Estimate actual error probability of MAP decoder */
    double pe_actual = 0.0;
    for (int j = 0; j < ny; j++) {
        double py = 0.0;
        for (int i = 0; i < nx; i++) py += joint_probs[i][j];
        if (py <= 0.0) continue;

        double max_p = 0.0;
        for (int i = 0; i < nx; i++)
            if (joint_probs[i][j] > max_p) max_p = joint_probs[i][j];

        pe_actual += (py - max_p);
    }

    return (lower - pe_actual); /* negative = Fano holds, positive = violation */
}

/* ── L2: Entropy Properties ─────────────────────────────────────── */
double entropy_max_uniform(int alphabet_size) {
    if (alphabet_size <= 1) return 0.0;
    return log2((double)alphabet_size);
}

double entropy_concavity_verify(const double* p, const double* q, int n, double lambda) {
    /* Verify H(λp + (1-λ)q) ≥ λH(p) + (1-λ)H(q)
     * Returns the difference: H_mixed - (λH(p) + (1-λ)H(q)).
     * Must be ≥ 0 (within numerical error). */
    if (!p || !q || n <= 0) return 0.0;

    double* mixed = malloc(n * sizeof(double));
    if (!mixed) return 0.0;

    for (int i = 0; i < n; i++)
        mixed[i] = lambda * p[i] + (1.0 - lambda) * q[i];

    double h_mixed = entropy_shannon(mixed, n);
    double h_comb = lambda * entropy_shannon(p, n) + (1.0 - lambda) * entropy_shannon(q, n);
    free(mixed);
    return h_mixed - h_comb;
}

double entropy_grouping_verify(const double* probs, int n, int split_point) {
    /* Verify grouping axiom: H(p₁,...,pₙ) = H(p₁+...+p_{k}, p_{k+1}+...+pₙ)
     *   + (p₁+...+p_{k})·H(p₁/Σ,...,p_{k}/Σ)
     *   + (p_{k+1}+...+pₙ)·H(p_{k+1}/Σ',...,pₙ/Σ')
     * Returns the absolute difference between LHS and RHS. */
    if (!probs || n <= 0 || split_point < 1 || split_point >= n) return 0.0;

    double sum1 = 0.0, sum2 = 0.0;
    for (int i = 0; i < split_point; i++) sum1 += probs[i];
    for (int i = split_point; i < n; i++) sum2 += probs[i];

    double h_total = entropy_shannon(probs, n);

    double group_probs[2] = {sum1, sum2};
    double h_grouped = entropy_shannon(group_probs, 2);

    double* cond1 = malloc(split_point * sizeof(double));
    double* cond2 = malloc((n - split_point) * sizeof(double));
    if (!cond1 || !cond2) { free(cond1); free(cond2); return 0.0; }

    for (int i = 0; i < split_point; i++)
        cond1[i] = sum1 > 0.0 ? probs[i] / sum1 : 0.0;
    for (int i = split_point; i < n; i++)
        cond2[i - split_point] = sum2 > 0.0 ? probs[i] / sum2 : 0.0;

    double h_cond1 = entropy_shannon(cond1, split_point);
    double h_cond2 = entropy_shannon(cond2, n - split_point);

    double rhs = h_grouped + sum1 * h_cond1 + sum2 * h_cond2;
    free(cond1); free(cond2);
    return fabs(h_total - rhs);
}

double entropy_gap_from_uniform(const double* probs, int n) {
    /* D(p||u) = log₂(n) - H(p) ≥ 0. Measures how far from uniform. */
    if (!probs || n <= 0) return 0.0;
    double h = entropy_shannon(probs, n);
    double max_h = log2((double)n);
    return max_h - h;
}
