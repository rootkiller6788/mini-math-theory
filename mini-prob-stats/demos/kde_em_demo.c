/* kde_em_demo.c ¡ª Kernel Density Estimation & EM Algorithm Demo
 *
 * Demonstrates:
 *   - Kernel Density Estimation (Silverman bandwidth)
 *   - EM Algorithm for Gaussian Mixture Model (2-component, 1D)
 *   - Bootstrap confidence intervals
 *
 * Application: clustering, density estimation, MLE with missing data
 */

#include "inference.h"
#include "distribution.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int main(void) {
    srand((unsigned)time(NULL));
    printf("=== KDE & EM Algorithm Demo ===\n\n");

    /* Generate mixed data: N(2, 0.5) + N(6, 0.5) */
    int n = 200;
    double* data = (double*)malloc(n * sizeof(double));
    if (!data) return 1;

    printf("Generating %d points from mixture: 0.5*N(2,0.5) + 0.5*N(6,0.5)\n\n", n);
    for (int i = 0; i < n; i++) {
        double u1 = (double)rand() / RAND_MAX;
        double u2 = (double)rand() / RAND_MAX;
        double z = sqrt(-2.0 * log(u1)) * cos(2.0 * PI * u2);
        if (i < n / 2) {
            data[i] = 2.0 + 0.5 * z;
        } else {
            data[i] = 6.0 + 0.5 * z;
        }
    }

    /* KDE */
    printf("--- Kernel Density Estimation ---\n");
    double bw = kde_bandwidth_silverman(data, n);
    printf("Silverman bandwidth: %.4f\n", bw);

    double test_points[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
    int n_test = 7;
    printf("\nKDE estimates:\n");
    for (int i = 0; i < n_test; i++) {
        double density = kde_evaluate(test_points[i], data, n, bw);
        printf("  f(%.1f) = %.6f\n", test_points[i], density);
    }

    /* EM Algorithm */
    printf("\n--- EM Algorithm (Gaussian Mixture, 2 components) ---\n");
    double pi[2] = {0.5, 0.5};
    double mu[2] = {1.0, 7.0};
    double sigma[2] = {1.0, 1.0};

    em_gaussian_mixture_1d(data, n, pi, mu, sigma, 100, 1e-6);

    printf("Fitted parameters:\n");
    printf("  Component 0: pi=%.4f, mu=%.4f, sigma=%.4f\n", pi[0], mu[0], sigma[0]);
    printf("  Component 1: pi=%.4f, mu=%.4f, sigma=%.4f\n", pi[1], mu[1], sigma[1]);
    printf("  (True: mu1=2.0, mu2=6.0, sigma1=sigma2=0.5, pi1=pi2=0.5)\n");

    /* Bootstrap CI */
    printf("\n--- Bootstrap 95%% CI for mean ---\n");
    double lo, hi;
    double sample_mean = bootstrap_ci(data, n, 1000, 0.95, &lo, &hi);
    printf("  Sample mean: %.4f, 95%% CI: [%.4f, %.4f]\n", sample_mean, lo, hi);

    free(data);
    return 0;
}
