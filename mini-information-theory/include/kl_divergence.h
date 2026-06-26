#ifndef KL_DIVERGENCE_H
#define KL_DIVERGENCE_H

#include <stdbool.h>

/* ── L1: Core Definitions ──────────────────────────────────────── */
/** L1: f-divergence type enum for generalized divergences */
typedef enum {
    FDIV_KL,         /* D_KL: f(t) = t log t */
    FDIV_REVERSE_KL, /* D_KL(Q||P): f(t) = -log t */
    FDIV_JS,         /* Jensen-Shannon: f(t) = -(t+1)log((t+1)/2) + t log t */
    FDIV_HELLINGER,  /* H²: f(t) = (√t - 1)² */
    FDIV_TV,         /* Total Variation: f(t) = |t-1| */
    FDIV_CHI2,       /* χ²: f(t) = (t-1)² */
    FDIV_ALPHA       /* α-divergence: generic parameterized */
} FDivergenceType;

/* ── L1: Core API ──────────────────────────────────────────────── */
double kl_divergence(const double* p, const double* q, int n);
double cross_entropy(const double* p, const double* q, int n);
double js_divergence(const double* p, const double* q, int n);
double kl_divergence_smoothed(const double* p, const double* q, int n, double eps);
double perplexity(const double* p, const double* q, int n);

/* ── L4: Pinsker's Inequality ──────────────────────────────────── */
/** L4: Pinsker's Inequality (Cover & Thomas, Lemma 11.6.1):
 *  D(P||Q) ≥ (1/(2 ln 2)) · ||P-Q||₁²
 *  Relates KL divergence to total variation distance.
 *  Returns the TV lower bound: sqrt(2·ln(2)·D_KL) ≤ ||P-Q||₁ */
double pinsker_bound_l1(const double* p, const double* q, int n);

/** L4: Total variation distance: ||P-Q||_TV = ½ Σ |pᵢ - qᵢ| */
double total_variation_distance(const double* p, const double* q, int n);

/** L4: Verify Pinsker: returns D_KL - (1/(2 ln 2))·||P-Q||₁² (must be ≥ 0) */
double pinsker_inequality_verify(const double* p, const double* q, int n);

/* ── L5: Renyi Divergence ───────────────────────────────────────── */
/** L5: Rényi divergence of order α:
 *  D_α(P||Q) = (1/(α-1)) log₂ Σ p_i^α q_i^{1-α}
 *  α→1 gives D_KL; α=0.5 gives Bhattacharyya-related; α=2 gives χ²-related. */
double renyi_divergence(const double* p, const double* q, int n, double alpha);

/* ── L2: Distance Measures ──────────────────────────────────────── */
/** L2: Bhattacharyya distance: -ln(Σ √(p_i q_i)) */
double bhattacharyya_distance(const double* p, const double* q, int n);

/** L2: Bhattacharyya coefficient: BC(p,q) = Σ √(p_i q_i) ∈ [0,1] */
double bhattacharyya_coefficient(const double* p, const double* q, int n);

/** L2: Hellinger distance: H(P,Q) = (1/√2) √(Σ (√p_i - √q_i)²) */
double hellinger_distance(const double* p, const double* q, int n);

/* ── L3: f-Divergence Framework ────────────────────────────────── */
/** L3: Generic f-divergence: D_f(P||Q) = Σ q_i f(p_i/q_i) */
double f_divergence(const double* p, const double* q, int n, FDivergenceType type);

/* ── L4: KL Divergence Properties ───────────────────────────────── */
/** L4: Non-negativity: D(P||Q) ≥ 0, equality iff P=Q (Gibbs' inequality).
 *  Returns the actual D_KL value for checking. */
bool kl_non_negative(const double* p, const double* q, int n);

/** L4: Asymmetry demonstration: D(P||Q) vs D(Q||P) */
double kl_asymmetry_ratio(const double* p, const double* q, int n);

/** L4: Convexity: D_KL is convex in the pair (P,Q).
 *  Verify D(λP₁+(1-λ)P₂ || λQ₁+(1-λ)Q₂) ≤ λ D(P₁||Q₁) + (1-λ) D(P₂||Q₂) */
double kl_convexity_verify(const double* p1, const double* q1,
                            const double* p2, const double* q2, int n, double lambda);

/** L4: Chain rule for KL divergence:
 *  D(P(X,Y) || Q(X,Y)) = D(P(X)||Q(X)) + D(P(Y|X) || Q(Y|X)) */
double kl_chain_rule_verify(const double** pxy, const double** qxy, int nx, int ny);

/** L5: KL divergence for Gaussian distributions (analytic).
 *  D(N(μ₁,σ₁²) || N(μ₂,σ₂²)) = log(σ₂/σ₁) + (σ₁²+(μ₁-μ₂)²)/(2σ₂²) - ½ */
double kl_divergence_gaussian(double mu1, double var1, double mu2, double var2);

#endif
