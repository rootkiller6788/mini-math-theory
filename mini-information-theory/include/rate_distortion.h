#ifndef RATE_DISTORTION_H
#define RATE_DISTORTION_H

/* ── L8: Rate-Distortion Theory ────────────────────────────────── */
/** L8: Cover & Thomas, Ch. 10. Shannon's rate-distortion theorem.
 *  R(D) = min I(X; Ẋ) subject to E[d(X,Ẋ)] ≤ D.
 *
 *  This module implements:
 *    - Blahut-Arimoto for R(D)
 *    - Lloyd-Max quantizer design
 *    - Analytic R(D) for binary, Gaussian, Laplacian sources
 */

/** L8: Rate-distortion result container */
typedef struct {
    double rate;                  /* R(D) in bits */
    double distortion;            /* achieved distortion D */
    double* input_distribution;   /* optimal p(x) */
    double** test_channel;        /* optimal p(ẋ|x) */
    int     iterations;           /* number of BA iterations */
} RDResult;

/* ── L8: Blahut-Arimoto for Rate-Distortion ────────────────────── */
/** L8: Compute R(D) by Blahut-Arimoto, parameterized by slope s < 0.
 *  Minimize I(X;Ẋ) + s·E[d(X,Ẋ)] → parametric curve of R(D).
 *  distortion_matrix[x][xhat]: d(x, ẋ) for each pair
 *  source_dist[x]: fixed source distribution p(x)
 *  n_x, n_xhat: input/output alphabet sizes
 *  slope: trade-off parameter (more negative → lower D, higher R)
 *  beta: Blahut-Arimoto cooling parameter (typically → 1) */
RDResult* rd_blahut_arimoto(const double** distortion_matrix,
                             const double* source_dist,
                             int n_x, int n_xhat,
                             double slope, int max_iter);
void rd_result_free(RDResult* result);

/* ── L8: Analytic Rate-Distortion Functions ────────────────────── */
/** L8: Binary source Ber(p) with Hamming distortion:
 *  R(D) = H(p) - H(D) for 0 ≤ D ≤ min(p, 1-p) */
double rd_binary_hamming(double p, double D);

/** L8: Gaussian N(0,σ²) with squared error:
 *  R(D) = ½ log₂(σ²/D) for 0 < D ≤ σ² */
double rd_gaussian_squared_error(double variance, double D);

/** L8: Shannon lower bound for difference distortion measures */
double rd_shannon_lower_bound_gaussian(double variance, double D);

/** L8: Laplacian source with absolute error distortion */
double rd_laplacian_absolute_error(double b, double D);

/* ── L5: Lloyd-Max Quantizer ──────────────────────────────────── */
/** L5: Lloyd-Max algorithm for optimal scalar quantizer design.
 *  pdf(x, params): source probability density function
 *  a, b: support bounds
 *  n_levels: number of quantization regions
 *  levels[]: output — reconstruction levels (size n_levels)
 *  thresholds[]: output — decision thresholds (size n_levels+1)
 *  Returns final MSE distortion. */
double lloyd_max_quantizer(double (*pdf)(double x, const void* params),
                            const void* params,
                            double a, double b, int n_levels,
                            double* levels, double* thresholds, int max_iter);

/** L5: Uniform quantizer MSE: Δ²/12 where Δ = 1/2^R */
double uniform_quantizer_mse(int n_bits);

/** L5: Gaussian quantizer SNR upper bound: 6.02R - 4.35 dB for high rates */
double gaussian_quantizer_mse(int n_bits);

/** L8: Entropy of quantizer output bins for rate computation */
double entropy_constrained_quantizer_rate(const double* bin_probs, int n_bins);

/* ── L6: Quantizer Benchmark ──────────────────────────────────── */
typedef struct {
    double snr_db;
    double rate;
    double distortion;
    double rd_bound_rate;
    double gap_to_bound;
} QuantizerBenchmark;

QuantizerBenchmark quantizer_benchmark(double snr_db, int n_bits,
                                         double source_variance);

#endif
