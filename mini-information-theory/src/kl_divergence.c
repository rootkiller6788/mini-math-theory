/* kl_divergence.c — KL Divergence, Pinsker, Rényi, f-Divergences
 * Reference: Cover & Thomas, Ch. 2.6, 11.6
 * MIT 6.441: Weeks 3-4
 *
 * Key formulas:
 *   D(P||Q) = Σ p(x) log₂(p(x)/q(x))              [Def 2.41]
 *   H(P,Q)  = H(P) + D(P||Q)                       [Eq 2.47]
 *   D(P||Q) ≥ (1/(2 ln 2)) ||P-Q||₁²               [Pinsker, Lemma 11.6.1]
 *   R_α(P||Q) = (1/(α-1)) log Σ pᵢ^α qᵢ^{1-α}     [Rényi, 1961]
 *   JSD(P,Q) = ½ D(P||M) + ½ D(Q||M)               [Lin 1991]
 */
#include "kl_divergence.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif

/* ── L1: Core Kullback-Leibler Divergence ───────────────────────── */
double kl_divergence(const double* p, const double* q, int n) {
    if (!p || !q || n <= 0) return 0.0;
    double d = 0.0;
    for (int i = 0; i < n; i++)
        if (p[i] > 0.0 && q[i] > 0.0)
            d += p[i] * log2(p[i] / q[i]);
    return d;
}

double cross_entropy(const double* p, const double* q, int n) {
    if (!p || !q || n <= 0) return 0.0;
    double h = 0.0;
    for (int i = 0; i < n; i++)
        if (q[i] > 0.0)
            h -= p[i] * log2(q[i]);
    return h;
}

double js_divergence(const double* p, const double* q, int n) {
    if (!p || !q || n <= 0) return 0.0;
    double* m = malloc(n * sizeof(double));
    if (!m) return 0.0;
    for (int i = 0; i < n; i++)
        m[i] = (p[i] + q[i]) / 2.0;

    double js = 0.5 * kl_divergence(p, m, n) + 0.5 * kl_divergence(q, m, n);
    free(m);
    return js;
}

double kl_divergence_smoothed(const double* p, const double* q, int n, double eps) {
    if (!p || !q || n <= 0) return 0.0;
    double* ps = malloc(n * sizeof(double));
    double* qs = malloc(n * sizeof(double));
    if (!ps || !qs) { free(ps); free(qs); return 0.0; }

    double np = 0, nq = 0;
    for (int i = 0; i < n; i++) {
        ps[i] = p[i] + eps;
        np += ps[i];
        qs[i] = q[i] + eps;
        nq += qs[i];
    }
    for (int i = 0; i < n; i++) { ps[i] /= np; qs[i] /= nq; }
    double d = kl_divergence(ps, qs, n);
    free(ps); free(qs);
    return d;
}

double perplexity(const double* p, const double* q, int n) {
    if (!p || !q || n <= 0) return 0.0;
    double ce = cross_entropy(p, q, n);
    return pow(2.0, ce);
}

/* ── L4: Pinsker's Inequality ───────────────────────────────────── */
double total_variation_distance(const double* p, const double* q, int n) {
    if (!p || !q || n <= 0) return 0.0;
    double tv = 0.0;
    for (int i = 0; i < n; i++)
        tv += fabs(p[i] - q[i]);
    return 0.5 * tv;
}

double pinsker_bound_l1(const double* p, const double* q, int n) {
    /* From Pinsker: D(P||Q) ≥ (1/(2 ln 2)) ||P-Q||₁²
     * So ||P-Q||₁ ≤ √(2·ln 2·D_KL)
     * Returns the upper bound on L1 distance from KL. */
    double dkl = kl_divergence(p, q, n);
    return sqrt(2.0 * M_LN2 * dkl);
}

double pinsker_inequality_verify(const double* p, const double* q, int n) {
    /* Return D_KL - (1/(2 ln 2))·||P-Q||₁². Must be ≥ 0. */
    if (!p || !q || n <= 0) return 0.0;
    double dkl = kl_divergence(p, q, n);
    double l1 = 0.0;
    for (int i = 0; i < n; i++)
        l1 += fabs(p[i] - q[i]);
    double pinsker_bound = l1 * l1 / (2.0 * M_LN2);
    return dkl - pinsker_bound;
}

/* ── L5: Rényi Divergence ──────────────────────────────────────── */
double renyi_divergence(const double* p, const double* q, int n, double alpha) {
    /* D_α(P||Q) = (1/(α-1)) log₂ Σ p_i^α q_i^{1-α}
     * α=1 limit → D_KL; α=0 → -log₂ Σ q_i 1_{p_i>0} */
    if (!p || !q || n <= 0) return 0.0;

    if (fabs(alpha - 1.0) < 1e-10) {
        return kl_divergence(p, q, n);
    }

    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        if (p[i] <= 0.0 || q[i] <= 0.0) continue;
        sum += pow(p[i], alpha) * pow(q[i], 1.0 - alpha);
    }

    if (sum <= 0.0) return 0.0;
    return log2(sum) / (alpha - 1.0);
}

/* ── L2: Distance Measures ──────────────────────────────────────── */
double bhattacharyya_coefficient(const double* p, const double* q, int n) {
    /* BC(p,q) = Σ √(p_i q_i). Related to Hellinger: H² = 1 - BC. */
    if (!p || !q || n <= 0) return 0.0;
    double bc = 0.0;
    for (int i = 0; i < n; i++)
        bc += sqrt(p[i] * q[i]);
    return bc;
}

double bhattacharyya_distance(const double* p, const double* q, int n) {
    /* BD(p,q) = -ln(BC(p,q)) */
    double bc = bhattacharyya_coefficient(p, q, n);
    if (bc <= 0.0) return 1e10;
    return -log(bc);
}

double hellinger_distance(const double* p, const double* q, int n) {
    /* H(P,Q) = (1/√2) √(Σ (√p_i - √q_i)²) = √(1 - BC(p,q)) */
    if (!p || !q || n <= 0) return 0.0;
    double bc = bhattacharyya_coefficient(p, q, n);
    double val = 1.0 - bc;
    if (val < 0.0) val = 0.0;
    return sqrt(val);
}

/* ── L3: f-Divergence Framework ─────────────────────────────────── */
static double fdiv_f(double t, FDivergenceType type) {
    switch (type) {
        case FDIV_KL:         return t > 0 ? t * log(t) : 0.0;
        case FDIV_REVERSE_KL: return t > 0 ? -log(t) : 1e10;
        case FDIV_JS:
            return t > 0 ? -(t + 1.0) * log((t + 1.0) / 2.0) + t * log(t) : log(2.0);
        case FDIV_HELLINGER: { double s = sqrt(t); return (s - 1.0) * (s - 1.0); }
        case FDIV_TV:         return fabs(t - 1.0);
        case FDIV_CHI2:       return (t - 1.0) * (t - 1.0);
        case FDIV_ALPHA:      return t > 0 ? t * log(t) : 0.0; /* default to KL */
        default:              return 0.0;
    }
}

double f_divergence(const double* p, const double* q, int n, FDivergenceType type) {
    /* D_f(P||Q) = Σ q_i f(p_i / q_i)
     * Reference: Csiszár 1967, Ali-Silvey 1966 */
    if (!p || !q || n <= 0) return 0.0;

    double div = 0.0;
    for (int i = 0; i < n; i++) {
        if (q[i] <= 0.0) {
            if (p[i] > 0.0) return 1e10; /* pᵢ > 0 but qᵢ = 0 → divergence = ∞ for KL-like */
            continue;
        }
        double t = p[i] / q[i];
        div += q[i] * fdiv_f(t, type);
    }
    return div;
}

/* ── L4: KL Divergence Properties ───────────────────────────────── */
bool kl_non_negative(const double* p, const double* q, int n) {
    /* Gibbs' inequality: D(P||Q) ≥ 0, with equality iff P = Q.
     * Proof: log x ≤ x - 1 for x > 0 ⇒ -D(P||Q) = Σ p log(q/p) ≤ Σ p(q/p - 1) = 0. */
    if (!p || !q || n <= 0) return false;
    double d = kl_divergence(p, q, n);
    return d >= -1e-12;
}

double kl_asymmetry_ratio(const double* p, const double* q, int n) {
    /* D(P||Q) / D(Q||P). Returns -1 if both are 0. */
    if (!p || !q || n <= 0) return 0.0;
    double d_pq = kl_divergence(p, q, n);
    double d_qp = kl_divergence(q, p, n);
    if (d_qp < 1e-15 && d_pq < 1e-15) return -1.0;
    if (d_qp < 1e-15) return 1e10;
    return d_pq / d_qp;
}

double kl_convexity_verify(const double* p1, const double* q1,
                            const double* p2, const double* q2, int n, double lambda) {
    /* Verify: D(λP₁+(1-λ)P₂ || λQ₁+(1-λ)Q₂) ≤ λ D(P₁||Q₁) + (1-λ) D(P₂||Q₂)
     * Returns: λD(P₁||Q₁) + (1-λ)D(P₂||Q₂) - D(mixed||mixed) — must be ≥ 0. */
    if (!p1 || !q1 || !p2 || !q2 || n <= 0) return 0.0;

    double* pmix = malloc(n * sizeof(double));
    double* qmix = malloc(n * sizeof(double));
    if (!pmix || !qmix) { free(pmix); free(qmix); return 0.0; }

    for (int i = 0; i < n; i++) {
        pmix[i] = lambda * p1[i] + (1.0 - lambda) * p2[i];
        qmix[i] = lambda * q1[i] + (1.0 - lambda) * q2[i];
    }

    double d_mix = kl_divergence(pmix, qmix, n);
    double d_comb = lambda * kl_divergence(p1, q1, n) + (1.0 - lambda) * kl_divergence(p2, q2, n);
    free(pmix); free(qmix);
    return d_comb - d_mix;
}

double kl_chain_rule_verify(const double** pxy, const double** qxy, int nx, int ny) {
    /* D(P(X,Y) || Q(X,Y)) = D(P(X)||Q(X)) + D(P(Y|X) || Q(Y|X))
     * where D(P(Y|X) || Q(Y|X)) = Σ_x p(x) D(P(Y|X=x) || Q(Y|X=x)) */
    if (!pxy || !qxy || nx <= 0 || ny <= 0) return 0.0;

    /* Compute marginals */
    double* px = calloc(nx, sizeof(double));
    double* qx = calloc(nx, sizeof(double));
    if (!px || !qx) { free(px); free(qx); return 0.0; }

    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++) {
            px[i] += pxy[i][j];
            qx[i] += qxy[i][j];
        }

    double d_joint = 0.0;
    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
            if (pxy[i][j] > 0.0 && qxy[i][j] > 0.0)
                d_joint += pxy[i][j] * log2(pxy[i][j] / qxy[i][j]);

    double d_marginal = 0.0;
    for (int i = 0; i < nx; i++)
        if (px[i] > 0.0 && qx[i] > 0.0)
            d_marginal += px[i] * log2(px[i] / qx[i]);

    double d_conditional = 0.0;
    for (int i = 0; i < nx; i++) {
        if (px[i] <= 0.0) continue;
        for (int j = 0; j < ny; j++) {
            double p_ygx = (px[i] > 0.0) ? pxy[i][j] / px[i] : 0.0;
            double q_ygx = (qx[i] > 0.0) ? qxy[i][j] / qx[i] : 0.0;
            if (p_ygx > 0.0 && q_ygx > 0.0)
                d_conditional += px[i] * p_ygx * log2(p_ygx / q_ygx);
        }
    }

    free(px); free(qx);
    return fabs(d_joint - (d_marginal + d_conditional));
}

double kl_divergence_gaussian(double mu1, double var1, double mu2, double var2) {
    /* D(N(μ₁,σ₁²) || N(μ₂,σ₂²)) = ½[log(σ₂²/σ₁²) + σ₁²/σ₂² + (μ₁-μ₂)²/σ₂² - 1]
     * Using natural log → convert to log₂ by dividing by ln(2). */
    if (var1 <= 0.0 || var2 <= 0.0) return 0.0;
    double diff = mu1 - mu2;
    double term = log(var2 / var1) + var1 / var2 + (diff * diff) / var2 - 1.0;
    return 0.5 * term / M_LN2;
}
