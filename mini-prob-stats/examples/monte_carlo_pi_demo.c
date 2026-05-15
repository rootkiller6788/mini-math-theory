#include "monte_carlo.h"
#include "distribution.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

static double f_x2(double x) {
    return x * x;
}

static double integrand_x2exp(double x) {
    return x * x * exp(-x);
}

static double g_exp_pdf(double x) {
    return exp(-x);
}

static double g_exp_sample(void) {
    double u = (double)rand() / RAND_MAX;
    return -log(1.0 - u);
}

static double target_normal(double x) {
    return exp(-0.5 * x * x);
}

int main(void) {
    srand((unsigned)time(NULL));

    printf("=== Monte Carlo Pi Estimation ===\n\n");

    int sample_counts[] = {100, 1000, 10000, 100000, 1000000};
    int n_levels = 5;

    for (int i = 0; i < n_levels; i++) {
        int ns = sample_counts[i];
        double pi_est = mc_pi(ns);
        double error = fabs(pi_est - PI);
        printf("Samples: %7d  Pi estimate: %.6f  Error: %.6f\n", ns, pi_est, error);
    }

    printf("\n=== Monte Carlo Integration: ∫ x^2 dx on [0,1] ===\n\n");
    for (int i = 0; i < n_levels; i++) {
        int ns = sample_counts[i];
        double integral = mc_integrate(f_x2, 0.0, 1.0, ns);
        double exact = 1.0 / 3.0;
        double error = fabs(integral - exact);
        printf("Samples: %7d  Integral: %.6f  Error: %.6f  (exact: %.6f)\n",
               ns, integral, error, exact);
    }

    printf("\n=== Importance Sampling: ∫ x^2 * e^{-x} dx on [0,∞) ===\n\n");
    for (int i = 0; i < n_levels; i++) {
        int ns = sample_counts[i];
        double estimate = mc_importance_sampling(integrand_x2exp, g_exp_pdf, g_exp_sample, ns);
        double exact = 2.0;
        double error = fabs(estimate - exact);
        printf("Samples: %7d  Estimate: %.6f  Error: %.6f  (exact: %.6f)\n",
               ns, estimate, error, exact);
    }

    printf("\n=== MCMC Metropolis-Hastings ===\n\n");
    int mcmc_n = 100000;
    double* mcmc_samples = (double*)malloc(mcmc_n * sizeof(double));
    mcmc_metropolis(target_normal, 0.0, mcmc_n, mcmc_samples, 1.0);

    double mcmc_mean = 0.0, mcmc_var = 0.0;
    for (int i = 1000; i < mcmc_n; i++) {
        mcmc_mean += mcmc_samples[i];
    }
    mcmc_mean /= (mcmc_n - 1000);
    for (int i = 1000; i < mcmc_n; i++) {
        double d = mcmc_samples[i] - mcmc_mean;
        mcmc_var += d * d;
    }
    mcmc_var /= (mcmc_n - 1000 - 1);

    printf("MCMC targeting N(0,1) with %d iterations (burn-in: 1000)\n", mcmc_n);
    printf("  Sample mean:     %.6f (expected 0.0)\n", mcmc_mean);
    printf("  Sample variance: %.6f (expected 1.0)\n", mcmc_var);

    free(mcmc_samples);

    return 0;
}
