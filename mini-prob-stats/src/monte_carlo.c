/* monte_carlo.c — Monte Carlo methods: integration, sampling, MCMC.
 *
 * L5: Monte Carlo Integration (naive, importance sampling)
 * L5: MCMC (Metropolis-Hastings, Gibbs sampling)
 * L8: Variance reduction (antithetic variates, control variates, stratified)
 *
 * Theorem sources:
 *   Metropolis, Rosenbluth, Teller (1953)
 *   Hastings (1970) — Metropolis-Hastings algorithm
 *   Geman & Geman (1984) — Gibbs sampler
 *   Fishman (1996) — Monte Carlo: Concepts, Algorithms, Applications
 */

#include "monte_carlo.h"
#include "distribution.h"
#include <stdlib.h>
#include <math.h>

/* --------------------------------------------------------------------------
 * L5: Naive Monte Carlo Integration
 * I = ∫_a^b f(x) dx ≈ (b-a)/N · Σ f(xᵢ), xᵢ ~ Uniform(a,b)
 * Convergence: O(1/√N)
 * -------------------------------------------------------------------------- */
double mc_integrate(double (*f)(double), double a, double b, int n_samples) {
    if (n_samples <= 0) return 0.0;
    double sum = 0.0;
    double width = b - a;
    for (int i = 0; i < n_samples; i++) {
        double x = a + ((double)rand() / RAND_MAX) * width;
        sum += f(x);
    }
    return (width * sum) / n_samples;
}

/* --------------------------------------------------------------------------
 * L5: Monte Carlo π estimation (Buffon's needle proxy)
 * π/4 = fraction of points in unit circle
 * -------------------------------------------------------------------------- */
double mc_pi(int n_samples) {
    if (n_samples <= 0) return 0.0;
    int inside = 0;
    for (int i = 0; i < n_samples; i++) {
        double x = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
        double y = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
        if (x * x + y * y <= 1.0) inside++;
    }
    return 4.0 * inside / n_samples;
}

/* --------------------------------------------------------------------------
 * L5: Importance Sampling
 * I = ∫ f(x) dx = ∫ (f(x)/g(x)) g(x) dx ≈ 1/N Σ f(xᵢ)/g(xᵢ)
 * where xᵢ ~ g(x). Good proposal g(x) ∝ f(x) for variance reduction.
 * -------------------------------------------------------------------------- */
double mc_importance_sampling(double (*f)(double), double (*g)(double),
                              double (*g_sample)(void), int n_samples) {
    if (n_samples <= 0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < n_samples; i++) {
        double x = g_sample();
        double gx = g(x);
        if (gx > 0.0) {
            sum += f(x) / gx;
        }
    }
    return sum / n_samples;
}

/* --------------------------------------------------------------------------
 * L5: Metropolis-Hastings algorithm
 *
 * Target distribution π(x) (unnormalized).
 * Proposal distribution q(x'|x) = N(x, proposal_sd²).
 * Accept with probability: α = min(1, π(x')q(x|x') / (π(x)q(x'|x)))
 *
 * For symmetric proposal (Metropolis): α = min(1, π(x')/π(x))
 * Complexity: O(N_iter)
 * -------------------------------------------------------------------------- */
void mcmc_metropolis(double (*target)(double), double start, int n_iter,
                     double* samples, double proposal_sd) {
    if (!samples || n_iter <= 0) return;
    double current = start;
    double target_current = target(current);

    for (int i = 0; i < n_iter; i++) {
        /* Box-Muller for Normal proposal */
        double u1 = (double)rand() / RAND_MAX;
        double u2 = (double)rand() / RAND_MAX;
        double proposal = current + proposal_sd * sqrt(-2.0 * log(u1)) * cos(2.0 * PI * u2);

        double target_proposal = target(proposal);
        double alpha = 1.0;
        if (target_current > 0.0) {
            alpha = target_proposal / target_current;
        }
        if (alpha > 1.0) alpha = 1.0;

        if ((double)rand() / RAND_MAX < alpha) {
            current = proposal;
            target_current = target_proposal;
        }
        samples[i] = current;
    }
}

/* --------------------------------------------------------------------------
 * L8: Gibbs Sampling (two-variable, bivariate normal target)
 *
 * Target: (x,y) ~ N(0, [1 ρ; ρ 1])
 * Full conditionals:
 *   x|y ~ N(ρ·y, 1-ρ²)
 *   y|x ~ N(ρ·x, 1-ρ²)
 *
 * Samples from a 2D multivariate normal with correlation ρ.
 * Complexity: O(2·N_iter)
 * -------------------------------------------------------------------------- */
void mcmc_gibbs_bivariate_normal(int n_iter, double rho, double* samples_x,
                                  double* samples_y) {
    if (!samples_x || !samples_y || n_iter <= 0) return;
    double x = 0.0, y = 0.0;
    double sqrt_1mr2 = sqrt(1.0 - rho * rho);

    for (int i = 0; i < n_iter; i++) {
        /* Sample x|y — Box-Muller with guard against u1=0 */
        double u1, u2, z1, z2;
        u1 = ((double)rand() + 1.0) / (RAND_MAX + 2.0);
        u2 = ((double)rand() + 1.0) / (RAND_MAX + 2.0);
        z1 = sqrt(-2.0 * log(u1)) * cos(2.0 * PI * u2);
        x = rho * y + sqrt_1mr2 * z1;

        /* Sample y|x */
        u1 = ((double)rand() + 1.0) / (RAND_MAX + 2.0);
        u2 = ((double)rand() + 1.0) / (RAND_MAX + 2.0);
        z2 = sqrt(-2.0 * log(u1)) * cos(2.0 * PI * u2);
        y = rho * x + sqrt_1mr2 * z2;

        samples_x[i] = x;
        samples_y[i] = y;
    }
}

/* --------------------------------------------------------------------------
 * L8: Antithetic Variates — variance reduction technique
 *
 * For each uniform U, also use 1-U.
 * If f is monotonic, Cov(f(U), f(1-U)) < 0 → variance reduction.
 *
 * I_AV = 1/(2N) Σ [f(Uᵢ) + f(1-Uᵢ)]
 * Complexity: O(2·N)
 * -------------------------------------------------------------------------- */
double mc_antithetic_integrate(double (*f)(double), double a, double b,
                                int n_samples) {
    if (n_samples <= 0) return 0.0;
    double sum = 0.0;
    double width = b - a;
    for (int i = 0; i < n_samples; i++) {
        double u = (double)rand() / RAND_MAX;
        double x1 = a + u * width;
        double x2 = a + (1.0 - u) * width;
        sum += f(x1) + f(x2);
    }
    return (width * sum) / (2.0 * n_samples);
}

/* --------------------------------------------------------------------------
 * L8: Control Variates — variance reduction using a correlated control
 *
 * Control variate h(x) with known expectation E[h].
 * I_cv = 1/N Σ [f(xᵢ) + β(E[h] - h(xᵢ))]
 *
 * Optimal β* = Cov(f, h) / Var(h)
 * For integral ∫ sin(x) dx on [0,π], use h(x) = x with E[h]=π/2
 * -------------------------------------------------------------------------- */
double mc_control_variate_integrate(double (*f)(double), double a, double b,
                                     int n_samples, double true_mean_h) {
    if (n_samples <= 0) return 0.0;
    double width = b - a;
    double sum_f = 0.0, sum_h = 0.0;
    double* xs = (double*)malloc(n_samples * sizeof(double));
    double* fs = (double*)malloc(n_samples * sizeof(double));
    double* hs = (double*)malloc(n_samples * sizeof(double));
    if (!xs || !fs || !hs) {
        free(xs); free(fs); free(hs);
        return mc_integrate(f, a, b, n_samples);
    }

    for (int i = 0; i < n_samples; i++) {
        xs[i] = a + ((double)rand() / RAND_MAX) * width;
        fs[i] = f(xs[i]);
        hs[i] = xs[i];  /* control variate: identity */
        sum_f += fs[i];
        sum_h += hs[i];
    }

    double mean_f = sum_f / n_samples;
    double mean_h = sum_h / n_samples;

    /* Cov(f, h) and Var(h) */
    double cov = 0.0, var_h = 0.0;
    for (int i = 0; i < n_samples; i++) {
        cov += (fs[i] - mean_f) * (hs[i] - mean_h);
        var_h += (hs[i] - mean_h) * (hs[i] - mean_h);
    }

    double beta_star = (var_h > 0.0) ? cov / var_h : 0.0;
    double result = width * (mean_f + beta_star * (true_mean_h - mean_h));

    free(xs); free(fs); free(hs);
    return result;
}

/* --------------------------------------------------------------------------
 * L5: Stratified Monte Carlo integration
 *
 * Divide [a,b] into K strata, sample evenly within each.
 * Reduces variance compared to naive MC.
 * -------------------------------------------------------------------------- */
double mc_stratified_integrate(double (*f)(double), double a, double b,
                                int n_samples, int n_strata) {
    if (n_samples <= 0 || n_strata <= 0) return 0.0;
    double total = 0.0;
    double width = (b - a) / n_strata;
    int per_stratum = n_samples / n_strata;
    if (per_stratum < 1) per_stratum = 1;

    for (int s = 0; s < n_strata; s++) {
        double a_s = a + s * width;
        double sum = 0.0;
        for (int i = 0; i < per_stratum; i++) {
            double x = a_s + ((double)rand() / RAND_MAX) * width;
            sum += f(x);
        }
        total += sum / per_stratum;
    }
    return (b - a) * total / n_strata;
}

/* --------------------------------------------------------------------------
 * L7: Portfolio risk simulation (VaR — Value at Risk)
 *
 * Simulate portfolio return r = Σ wᵢ·log(S_T/S_0) with correlated assets.
 * Geometric Brownian Motion: S_T = S_0 exp((μ-σ²/2)T + σ√T·Z)
 * VaR_α = (1-α)-quantile of negative returns
 *
 * Application: Financial risk assessment, Basel III compliance
 * -------------------------------------------------------------------------- */
double mc_portfolio_var(double* weights, double* returns, double* volatilities,
                         double* correlation, int n_assets, int n_sim,
                         double alpha) {
    if (n_assets <= 0 || n_sim <= 0) return 0.0;

    /* Cholesky for 2-asset case only */
    double L[4];
    if (n_assets == 2) {
        L[0] = 1.0;
        L[1] = 0.0;
        L[2] = correlation[0];
        double rho = correlation[0];
        L[3] = sqrt(1.0 - rho * rho);
    } else {
        /* Independent assets for n>2 */
        for (int i = 0; i < n_assets * n_assets; i++) L[i] = 0.0;
        for (int i = 0; i < n_assets; i++) L[i * n_assets + i] = 1.0;
    }

    double* sim_returns = (double*)malloc(n_sim * sizeof(double));
    if (!sim_returns) return 0.0;

    for (int s = 0; s < n_sim; s++) {
        double portfolio_return = 0.0;
        for (int i = 0; i < n_assets; i++) {
            /* Generate correlated normal via Cholesky */
            double z = 0.0;
            for (int j = 0; j < n_assets; j++) {
                double u1 = (double)rand() / RAND_MAX;
                double u2 = (double)rand() / RAND_MAX;
                double zj = sqrt(-2.0 * log(u1)) * cos(2.0 * PI * u2);
                z += L[i * n_assets + j] * zj;
            }
            double drift = returns[i] - 0.5 * volatilities[i] * volatilities[i];
            double asset_return = drift + volatilities[i] * z;
            portfolio_return += weights[i] * asset_return;
        }
        sim_returns[s] = -portfolio_return;  /* negative returns for VaR */
    }

    /* Sort and find (1-α)-quantile */
    for (int i = 0; i < n_sim - 1; i++)
        for (int j = i + 1; j < n_sim; j++)
            if (sim_returns[j] < sim_returns[i]) {
                double t = sim_returns[i]; sim_returns[i] = sim_returns[j]; sim_returns[j] = t;
            }

    int idx = (int)((1.0 - alpha) * n_sim);
    if (idx < 0) idx = 0;
    if (idx >= n_sim) idx = n_sim - 1;
    double var_value = sim_returns[idx];

    free(sim_returns);
    return var_value;
}

/* --------------------------------------------------------------------------
 * L7: Monte Carlo power calculation for t-test
 *
 * Simulate power empirically: generate data under H₁, count rejections.
 * -------------------------------------------------------------------------- */
double mc_power_t_test(int n_sim, int n, double mu0, double mu1, double sigma,
                        double alpha) {
    if (n_sim <= 0 || n <= 1) return 0.0;
    int rejections = 0;

    for (int s = 0; s < n_sim; s++) {
        double sum = 0.0, sum_sq = 0.0;
        for (int i = 0; i < n; i++) {
            double u1 = (double)rand() / RAND_MAX;
            double u2 = (double)rand() / RAND_MAX;
            double z = sqrt(-2.0 * log(u1)) * cos(2.0 * PI * u2);
            double val = mu1 + sigma * z;
            sum += val;
            sum_sq += val * val;
        }
        double xbar = sum / n;
        double s2 = (sum_sq - sum * sum / n) / (n - 1);
        double se = sqrt(s2 / n);
        double t_stat = (xbar - mu0) / se;
        double p_val = 2.0 * (1.0 - 0.5 * (1.0 + erf(fabs(t_stat) / sqrt(2.0))));
        if (p_val < alpha) rejections++;
    }

    return (double)rejections / n_sim;
}
