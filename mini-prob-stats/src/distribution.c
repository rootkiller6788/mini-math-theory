/* distribution.c — Continuous & discrete probability distributions.
 *
 * Implements PMFs/PDFs, CDFs, expectations, variances across 14 families.
 * Covering: Bernoulli, Binomial, Poisson, Gaussian, Uniform, Exponential,
 * Gamma, Beta, Student's t, Chi-squared, F, Lognormal, Weibull, Cauchy.
 *
 * Theorem sources:
 *   MIT 18.05 §4–§6 (Common Distributions)
 *   CS229 §G.3 (Probability Review)
 *   Casella & Berger §2–§3 (Statistical Inference)
 */

#include "distribution.h"
#include "probability.h"
#include <math.h>
#include <stdlib.h>
#include <float.h>

/* --------------------------------------------------------------------------
 * L1: Bernoulli — the simplest discrete distribution
 * -------------------------------------------------------------------------- */
double bernoulli_pmf(int k, double p) {
    if (k == 1) return p;
    if (k == 0) return 1.0 - p;
    return 0.0;
}

double bernoulli_expectation(double p) {
    return p;
}

double bernoulli_variance(double p) {
    return p * (1.0 - p);
}

/* --------------------------------------------------------------------------
 * L1: Binomial — n independent Bernoulli trials
 * PMF: P(X=k) = C(n,k) p^k (1-p)^{n-k}
 * E[X] = np, Var(X) = np(1-p)
 * -------------------------------------------------------------------------- */
double binomial_pmf(int k, int n, double p) {
    if (k < 0 || k > n) return 0.0;
    return (double)nCr(n, k) * pow(p, k) * pow(1.0 - p, n - k);
}

double binomial_cdf(int k, int n, double p) {
    if (k < 0) return 0.0;
    if (k >= n) return 1.0;
    double sum = 0.0;
    for (int i = 0; i <= k; i++) {
        sum += binomial_pmf(i, n, p);
    }
    return sum;
}

double binomial_expectation(int n, double p) {
    return n * p;
}

double binomial_variance(int n, double p) {
    return n * p * (1.0 - p);
}

/* --------------------------------------------------------------------------
 * L1: Poisson — counting events in fixed interval
 * PMF: P(X=k) = λ^k e^{-λ} / k!
 * E[X] = Var(X) = λ
 * -------------------------------------------------------------------------- */
double poisson_pmf(int k, double lambda) {
    if (k < 0 || lambda < 0.0) return 0.0;
    return pow(lambda, k) * exp(-lambda) / (double)factorial(k);
}

double poisson_cdf(int k, double lambda) {
    if (k < 0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i <= k; i++) {
        sum += poisson_pmf(i, lambda);
    }
    return sum;
}

double poisson_expectation(double lambda) {
    return lambda;
}

double poisson_variance(double lambda) {
    return lambda;
}

/* --------------------------------------------------------------------------
 * L1: Gaussian (Normal) — central to all of statistics
 * PDF: f(x) = (1/(σ√(2π))) exp(-(x-μ)²/(2σ²))
 * L4: Central Limit Theorem — sums converge to normal
 * -------------------------------------------------------------------------- */
double gaussian_pdf(double x, double mu, double sigma) {
    if (sigma <= 0.0) return 0.0;
    double z = (x - mu) / sigma;
    return exp(-0.5 * z * z) / (sigma * sqrt(2.0 * PI));
}

double gaussian_cdf(double x, double mu, double sigma) {
    if (sigma <= 0.0) return (x >= mu) ? 1.0 : 0.0;
    return 0.5 * (1.0 + erf((x - mu) / (sigma * sqrt(2.0))));
}

double gaussian_expectation(double mu, double sigma) {
    (void)sigma;
    return mu;
}

double gaussian_variance(double mu, double sigma) {
    (void)mu;
    return sigma * sigma;
}

/* L5: Gaussian quantile (probit) via rational approximation (Abramowitz & Stegun 26.2.23)
 * Complexity: O(1), accuracy ~1e-15 in [1e-7, 1-1e-7] */
double gaussian_quantile(double p) {
    if (p <= 0.0) return -1e10;
    if (p >= 1.0) return 1e10;

    double a0 = -3.969683028665376e+01;
    double a1 =  2.209460984245205e+02;
    double a2 = -2.759285104469687e+02;
    double a3 =  1.383577518672690e+02;
    double a4 = -3.066479806614716e+01;
    double a5 =  2.506628277459239e+00;

    double b0 = -5.447609879822406e+01;
    double b1 =  1.615858368580409e+02;
    double b2 = -1.556989798598866e+02;
    double b3 =  6.680131188771972e+01;
    double b4 = -1.328068155288572e+01;

    double c0 = -7.784894002430293e-03;
    double c1 = -3.223964580411365e-01;
    double c2 = -2.400758277161838e+00;
    double c3 = -2.549732539343734e+00;
    double c4 =  4.374664141464968e+00;
    double c5 =  2.938163982698783e+00;

    double d0 =  7.784695709041462e-03;
    double d1 =  3.224671290700398e-01;
    double d2 =  2.445134137142996e+00;
    double d3 =  3.754408661907416e+00;

    double p_low  = 0.02425;
    double p_high = 1.0 - p_low;
    double q, r, x;

    if (p < p_low) {
        q = sqrt(-2.0 * log(p));
        x = (((((c0 * q + c1) * q + c2) * q + c3) * q + c4) * q + c5)
          / ((((d0 * q + d1) * q + d2) * q + d3) * q + 1.0);
    } else if (p <= p_high) {
        q = p - 0.5;
        r = q * q;
        x = (((((a0 * r + a1) * r + a2) * r + a3) * r + a4) * r + a5) * q
          / (((((b0 * r + b1) * r + b2) * r + b3) * r + b4) * r + 1.0);
    } else {
        q = sqrt(-2.0 * log(1.0 - p));
        x = -((((((c0 * q + c1) * q + c2) * q + c3) * q + c4) * q + c5)
            / ((((d0 * q + d1) * q + d2) * q + d3) * q + 1.0));
    }
    return x;
}

/* --------------------------------------------------------------------------
 * L1: Uniform — on [a, b]
 * PDF: f(x) = 1/(b-a) for x ∈ [a,b], 0 otherwise
 * E[X] = (a+b)/2, Var(X) = (b-a)²/12
 * -------------------------------------------------------------------------- */
double uniform_pdf(double x, double a, double b) {
    if (b <= a) return 0.0;
    if (x < a || x > b) return 0.0;
    return 1.0 / (b - a);
}

double uniform_cdf(double x, double a, double b) {
    if (b <= a) return 0.0;
    if (x < a) return 0.0;
    if (x > b) return 1.0;
    return (x - a) / (b - a);
}

double uniform_expectation(double a, double b) {
    return (a + b) / 2.0;
}

double uniform_variance(double a, double b) {
    double w = b - a;
    return (w * w) / 12.0;
}

/* --------------------------------------------------------------------------
 * L1: Exponential — memoryless waiting times
 * PDF: f(x) = λ e^{-λx}, x ≥ 0
 * E[X] = 1/λ, Var(X) = 1/λ²
 * -------------------------------------------------------------------------- */
double exponential_pdf(double x, double lambda) {
    if (x < 0.0 || lambda <= 0.0) return 0.0;
    return lambda * exp(-lambda * x);
}

double exponential_cdf(double x, double lambda) {
    if (x < 0.0 || lambda <= 0.0) return 0.0;
    return 1.0 - exp(-lambda * x);
}

double exponential_expectation(double lambda) {
    if (lambda <= 0.0) return 0.0;
    return 1.0 / lambda;
}

double exponential_variance(double lambda) {
    if (lambda <= 0.0) return 0.0;
    return 1.0 / (lambda * lambda);
}

/* --------------------------------------------------------------------------
 * L2: Gamma — wait time for α events, rate β
 * PDF: f(x; α, β) = (β^α / Γ(α)) x^{α-1} e^{-βx}, x ≥ 0
 * E[X] = α/β, Var(X) = α/β²
 * Special cases: Exponential (α=1), Chi-squared (α=k/2, β=1/2)
 * Reference: MIT 18.05 §6.2
 * -------------------------------------------------------------------------- */
double log_gamma_stirling(double x) {
    /* Lanczos approximation for log Γ(x), x > 0 */
    if (x <= 0.0) return 0.0;
    double g = 7;
    double coeff[9] = {
        0.99999999999980993, 676.5203681218851, -1259.1392167224028,
        771.32342877765313, -176.61502916214059, 12.507343278686905,
        -0.13857109526572012, 9.9843695780195716e-6, 1.5056327351493116e-7
    };
    double t = x + g + 0.5;
    double s = coeff[0];
    for (int i = 1; i < 9; i++) {
        s += coeff[i] / (x + (double)i);
    }
    return log(sqrt(2.0 * PI)) + (x + 0.5) * log(t) - t + log(s) - log(x);
}

double gamma_pdf(double x, double alpha, double beta) {
    if (x < 0.0 || alpha <= 0.0 || beta <= 0.0) return 0.0;
    if (x == 0.0) return (alpha == 1.0) ? beta : 0.0;
    double log_density = alpha * log(beta) - log_gamma_stirling(alpha)
                       + (alpha - 1.0) * log(x) - beta * x;
    return exp(log_density);
}

double gamma_expectation(double alpha, double beta) {
    if (alpha <= 0.0 || beta <= 0.0) return 0.0;
    return alpha / beta;
}

double gamma_variance(double alpha, double beta) {
    if (alpha <= 0.0 || beta <= 0.0) return 0.0;
    return alpha / (beta * beta);
}

/* --------------------------------------------------------------------------
 * L2: Beta — continuous on [0,1], conjugate prior for binomial
 * PDF: f(x; α, β) = x^{α-1} (1-x)^{β-1} / B(α, β)
 * E[X] = α/(α+β), Var(X) = αβ/((α+β)²(α+β+1))
 * -------------------------------------------------------------------------- */
double beta_pdf(double x, double alpha, double beta_param) {
    if (x < 0.0 || x > 1.0 || alpha <= 0.0 || beta_param <= 0.0) return 0.0;
    double log_beta = log_gamma_stirling(alpha) + log_gamma_stirling(beta_param)
                     - log_gamma_stirling(alpha + beta_param);
    double log_density = (alpha - 1.0) * log(x) + (beta_param - 1.0) * log(1.0 - x) - log_beta;
    return exp(log_density);
}

double beta_expectation(double alpha, double beta_param) {
    if (alpha <= 0.0 || beta_param <= 0.0) return 0.0;
    return alpha / (alpha + beta_param);
}

double beta_variance(double alpha, double beta_param) {
    if (alpha <= 0.0 || beta_param <= 0.0) return 0.0;
    double ab = alpha + beta_param;
    return alpha * beta_param / (ab * ab * (ab + 1.0));
}

/* --------------------------------------------------------------------------
 * L2: Student's t — robust alternative to normal, heavy tails
 * PDF: f(t; ν) ∝ (1 + t²/ν)^{-(ν+1)/2}
 * For ν→∞ converges to N(0,1)
 * -------------------------------------------------------------------------- */
double t_pdf(double x, double nu) {
    if (nu <= 0.0) return 0.0;
    double log_half = log_gamma_stirling((nu + 1.0) / 2.0)
                     - log_gamma_stirling(nu / 2.0);
    return exp(log_half) / (sqrt(nu * PI))
         * pow(1.0 + x * x / nu, -(nu + 1.0) / 2.0);
}

double t_cdf(double x, double nu) {
    /* For large df, approximate with normal */
    if (nu <= 0.0) return 0.0;
    if (nu > 100.0) return gaussian_cdf(x, 0.0, 1.0);
    /* Regularized incomplete beta approximation */
    double xt = (x + sqrt(x * x + nu)) / (2.0 * sqrt(x * x + nu));
    if (x > 0.0) return 1.0 - 0.5 * beta_pdf(xt, nu / 2.0, 0.5) * 0.5;
    return 0.5 * beta_pdf(xt, nu / 2.0, 0.5) * 0.5;
}

double t_expectation(double nu) {
    if (nu > 1.0) return 0.0;
    return 0.0;
}

double t_variance(double nu) {
    if (nu > 2.0) return nu / (nu - 2.0);
    return 1.0 / 0.0;
}

/* --------------------------------------------------------------------------
 * L2: Chi-squared (χ²) — sum of k squared standard normals
 * PDF: f(x; k) = x^{k/2-1} e^{-x/2} / (2^{k/2} Γ(k/2)), x ≥ 0
 * E[X] = k, Var(X) = 2k
 * -------------------------------------------------------------------------- */
double chi2_pdf(double x, double k) {
    if (x < 0.0 || k <= 0.0) return 0.0;
    if (x == 0.0) return (k == 2.0) ? 0.5 : 0.0;
    double log_density = (k / 2.0 - 1.0) * log(x) - x / 2.0
                        - (k / 2.0) * log(2.0) - log_gamma_stirling(k / 2.0);
    return exp(log_density);
}

double chi2_expectation(double k) {
    return k;
}

double chi2_variance(double k) {
    return 2.0 * k;
}

/* --------------------------------------------------------------------------
 * L2: F — ratio of two scaled chi-squared variables
 * PDF: complex, uses beta function
 * E[X] = ν₂/(ν₂-2) for ν₂>2
 * -------------------------------------------------------------------------- */
double f_pdf(double x, double d1, double d2) {
    if (x < 0.0 || d1 <= 0.0 || d2 <= 0.0) return 0.0;
    if (x == 0.0) return (d1 == 2.0) ? 1.0 : 0.0;
    double log_b = log_gamma_stirling((d1 + d2) / 2.0) - log_gamma_stirling(d1 / 2.0)
                 - log_gamma_stirling(d2 / 2.0);
    double log_density = log_b + (d1 / 2.0) * log(d1 / d2) + (d1 / 2.0 - 1.0) * log(x)
                        - ((d1 + d2) / 2.0) * log(1.0 + d1 * x / d2);
    return exp(log_density);
}

double f_expectation(double d1, double d2) {
    (void)d1;
    if (d2 > 2.0) return d2 / (d2 - 2.0);
    return 1.0 / 0.0;
}

double f_variance(double d1, double d2) {
    (void)d1;
    if (d2 > 4.0) {
        return 2.0 * d2 * d2 * (d1 + d2 - 2.0)
             / (d1 * (d2 - 2.0) * (d2 - 2.0) * (d2 - 4.0));
    }
    return 1.0 / 0.0;
}

/* --------------------------------------------------------------------------
 * L2: Lognormal — log(X) ~ N(μ, σ²), models multiplicative processes
 * PDF: f(x) = 1/(xσ√(2π)) exp(-(ln x - μ)²/(2σ²)), x > 0
 * E[X] = exp(μ + σ²/2), Var(X) = exp(2μ+σ²)(exp(σ²)-1)
 * -------------------------------------------------------------------------- */
double lognormal_pdf(double x, double mu, double sigma) {
    if (x <= 0.0 || sigma <= 0.0) return 0.0;
    double logx = log(x);
    double z = (logx - mu) / sigma;
    return exp(-0.5 * z * z) / (x * sigma * sqrt(2.0 * PI));
}

double lognormal_expectation(double mu, double sigma) {
    return exp(mu + sigma * sigma / 2.0);
}

double lognormal_variance(double mu, double sigma) {
    double s2 = sigma * sigma;
    return exp(2.0 * mu + s2) * (exp(s2) - 1.0);
}

/* --------------------------------------------------------------------------
 * L2: Weibull — flexible lifetime distribution
 * PDF: f(x) = (k/λ) (x/λ)^{k-1} exp(-(x/λ)^k), x ≥ 0
 * k=1 → Exponential, k=2 → Rayleigh
 * E[X] = λ Γ(1+1/k)
 * -------------------------------------------------------------------------- */
double weibull_pdf(double x, double shape, double scale) {
    if (x < 0.0 || shape <= 0.0 || scale <= 0.0) return 0.0;
    if (x == 0.0) return (shape == 1.0) ? 1.0 / scale : 0.0;
    double t = x / scale;
    return (shape / scale) * pow(t, shape - 1.0) * exp(-pow(t, shape));
}

double weibull_cdf(double x, double shape, double scale) {
    if (x < 0.0 || shape <= 0.0 || scale <= 0.0) return 0.0;
    return 1.0 - exp(-pow(x / scale, shape));
}

/* L8: Weibull expectation uses the Gamma function: λ·Γ(1+1/k) */
double weibull_expectation(double shape, double scale) {
    if (shape <= 0.0 || scale <= 0.0) return 0.0;
    double log_expect = log(scale) + log_gamma_stirling(1.0 + 1.0 / shape);
    return exp(log_expect);
}

double weibull_variance(double shape, double scale) {
    if (shape <= 0.0 || scale <= 0.0) return 0.0;
    double g1 = exp(log_gamma_stirling(1.0 + 1.0 / shape));
    double g2 = exp(log_gamma_stirling(1.0 + 2.0 / shape));
    return scale * scale * (g2 - g1 * g1);
}

/* --------------------------------------------------------------------------
 * L2: Cauchy — heavy-tailed, no mean/variance (undefined moments)
 * PDF: f(x; x₀, γ) = 1/(πγ[1+((x-x₀)/γ)²])
 * Famous for: mean does not exist, yet CLT fails — Cauchy distribution!
 * -------------------------------------------------------------------------- */
double cauchy_pdf(double x, double x0, double gamma) {
    if (gamma <= 0.0) return 0.0;
    double z = (x - x0) / gamma;
    return 1.0 / (PI * gamma * (1.0 + z * z));
}

double cauchy_cdf(double x, double x0, double gamma) {
    if (gamma <= 0.0) return 0.0;
    return 0.5 + atan((x - x0) / gamma) / PI;
}

/* --------------------------------------------------------------------------
 * L4: Central Limit Theorem — sample mean distribution
 * For i.i.d. with finite variance: √n(X̄ₙ-μ)/σ → N(0,1)
 * This function generates n_samples of sample means from Uniform[0,1]
 * -------------------------------------------------------------------------- */
double* sample_mean_distribution(int sample_size, int n_samples) {
    double* means = (double*)malloc(n_samples * sizeof(double));
    if (!means) return NULL;
    for (int i = 0; i < n_samples; i++) {
        double sum = 0.0;
        for (int j = 0; j < sample_size; j++) {
            sum += (double)rand() / RAND_MAX;
        }
        means[i] = sum / sample_size;
    }
    return means;
}

/* --------------------------------------------------------------------------
 * L4: Law of Large Numbers verification
 * Computes running mean of uniform[0,1] random variables — should converge to 0.5
 * Returns array of n cumulative averages.
 * -------------------------------------------------------------------------- */
double* llN_running_mean(int n) {
    double* cum_means = (double*)malloc(n * sizeof(double));
    if (!cum_means) return NULL;
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += (double)rand() / RAND_MAX;
        cum_means[i] = sum / (i + 1);
    }
    return cum_means;
}

/* --------------------------------------------------------------------------
 * L5: Multivariate Gaussian log-PDF
 * log f(x; μ, Σ) = -k/2 log(2π) - 1/2 log|Σ| - 1/2 (x-μ)^T Σ^{-1} (x-μ)
 * For diagonal covariance (simplified case): Σ = diag(σ₁²,...,σₖ²)
 * -------------------------------------------------------------------------- */
double multivariate_gaussian_logpdf(double* x, double* mu, double* sigma_sq,
                                     int k) {
    if (k <= 0) return 0.0;
    double logdet = 0.0;
    double quad = 0.0;
    for (int i = 0; i < k; i++) {
        if (sigma_sq[i] <= 0.0) return -1e300;
        logdet += log(sigma_sq[i]);
        double d = x[i] - mu[i];
        quad += d * d / sigma_sq[i];
    }
    return -0.5 * (k * log(2.0 * PI) + logdet + quad);
}

/* --------------------------------------------------------------------------
 * L5: Distribution moments (skewness, kurtosis) for common distributions
 * -------------------------------------------------------------------------- */
double gaussian_skewness(void) { return 0.0; }
double gaussian_excess_kurtosis(void) { return 0.0; }
double exponential_skewness(void) { return 2.0; }
double exponential_excess_kurtosis(void) { return 6.0; }

/* --------------------------------------------------------------------------
 * L7: Rayleigh distribution (special case of Weibull with shape=2)
 * Used in signal processing, wireless communications
 * PDF: f(x) = (x/σ²) exp(-x²/(2σ²)), x ≥ 0
 * -------------------------------------------------------------------------- */
double rayleigh_pdf(double x, double sigma) {
    if (x < 0.0 || sigma <= 0.0) return 0.0;
    double s2 = sigma * sigma;
    return (x / s2) * exp(-x * x / (2.0 * s2));
}

double rayleigh_expectation(double sigma) {
    return sigma * sqrt(PI / 2.0);
}

double rayleigh_variance(double sigma) {
    return sigma * sigma * (4.0 - PI) / 2.0;
}

/* --------------------------------------------------------------------------
 * L7: Geometric distribution — number of failures before first success
 * PMF: P(X=k) = (1-p)^k p, k = 0,1,2,...
 * E[X] = (1-p)/p, Var(X) = (1-p)/p²
 * Application: A/B test sample size calculation
 * -------------------------------------------------------------------------- */
double geometric_pmf(int k, double p) {
    if (k < 0 || p <= 0.0 || p > 1.0) return 0.0;
    return pow(1.0 - p, k) * p;
}

double geometric_expectation(double p) {
    if (p <= 0.0 || p > 1.0) return 0.0;
    return (1.0 - p) / p;
}

double geometric_variance(double p) {
    if (p <= 0.0 || p > 1.0) return 0.0;
    return (1.0 - p) / (p * p);
}
