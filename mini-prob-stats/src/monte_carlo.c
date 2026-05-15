#include "monte_carlo.h"
#include "distribution.h"
#include <stdlib.h>
#include <math.h>

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

void mcmc_metropolis(double (*target)(double), double start, int n_iter,
                     double* samples, double proposal_sd) {
    if (!samples || n_iter <= 0) return;
    double current = start;
    double target_current = target(current);

    for (int i = 0; i < n_iter; i++) {
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
