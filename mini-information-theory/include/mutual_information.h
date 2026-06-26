#ifndef MUTUAL_INFORMATION_H
#define MUTUAL_INFORMATION_H

/* ── L1: Core Definitions ──────────────────────────────────────── */
/** L2: Triple joint distribution for conditional mutual information */
typedef struct {
    double*** pxyz;  /* pxyz[z][x][y] — probability mass indexed by Z first */
    int nx, ny, nz;
} TripleJoint;

/* ── L1: Core API ──────────────────────────────────────────────── */
double mutual_information(const double** pxy, const double* px, const double* py,
                          int nx, int ny);
double normalized_mutual_information(const double** pxy, const double* px,
                                     const double* py, int nx, int ny);
void   mutual_info_from_counts(const int** counts, int nx, int ny, double* mi);

/* ── L2: Conditional Mutual Information ────────────────────────── */
/** I(X;Y|Z) = Σ p(z) I(X;Y|Z=z) = H(X|Z) - H(X|Y,Z)
 *  Cover & Thomas, Definition 2.42 */
double conditional_mutual_information(const double*** pxyz, const double* pz,
                                      int nx, int ny, int nz);

/* ── L3: Chain Rule for Mutual Information ──────────────────────── */
/** I(X₁,X₂,...,Xₙ;Y) = Σᵢ I(Xᵢ;Y|X₁,...,Xᵢ₋₁)
 *  Cover & Thomas, Theorem 2.5.2 */
double mutual_info_chain_rule(const double*** joint_xyz, int nvars,
                               const double* pz, int nz);

/* ── L4: Data Processing Inequality ────────────────────────────── */
/** DPI: If X→Y→Z forms a Markov chain, then I(X;Z) ≤ I(X;Y).
 *  Cover & Thomas, Theorem 2.8.1.
 *  This function checks the DPI given the full joint P(X,Y,Z). */
bool data_processing_inequality_holds(const double*** pxyz, int nx, int ny, int nz);

/** Compute the DPI gap: I(X;Y) - I(X;Z) — must be ≥ 0 if X→Y→Z. */
double dpi_gap(const double*** pxyz, int nx, int ny, int nz);

/* ── L5: Interaction Information ───────────────────────────────── */
/** I(X;Y;Z) = I(X;Y) - I(X;Y|Z). Can be negative (synergy/redundancy).
 *  McGill 1954, Cover & Thomas, Sec 2.5 */
double interaction_information(const double*** pxyz, const double* px,
                                const double* py, const double* pz,
                                int nx, int ny, int nz);

/* ── L7: Transfer Entropy (Granger-style causality) ─────────────── */
/** T_{Y→X} = I(X_{t+1}; Y_t | X_t). Measures directed information flow.
 *  Schreiber 2000. Used in neuroscience, finance. */
double transfer_entropy(const int* x_series, const int* y_series, int length,
                         int x_states, int y_states);

/* ── L2: Properties ─────────────────────────────────────────────── */
/** L2: Symmetry — I(X;Y) = I(Y;X). Returns absolute difference (≈ 0). */
double mutual_info_symmetry_check(const double** pxy, const double* px,
                                   const double* py, int nx, int ny);

/** L2: Non-negativity — I(X;Y) ≥ 0. */
bool mutual_info_non_negative(const double** pxy, const double* px,
                               const double* py, int nx, int ny);

/** L4: Fano metric — d_F(X,Y) = H(X|Y) + H(Y|X). Symmetric distance. */
double fano_metric(const double** joint_probs, int nx, int ny);

/** L5: Variation of Information — VI(X,Y) = H(X|Y) + H(Y|X)
 *  = H(X) + H(Y) - 2I(X;Y). Metric on probability distributions. */
double variation_of_information(const double** pxy, const double* px,
                                 const double* py, int nx, int ny);

#endif
