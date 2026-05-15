#include "queue_model.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

static double uniform_rand(void) {
    return (double)rand() / (double)RAND_MAX;
}

static double exponential_rand(double rate) {
    double u = uniform_rand();
    if (u < 1e-10) u = 1e-10;
    return -log(u) / rate;
}

int main(void) {
    srand((unsigned)time(NULL));
    printf("=== Queue Theory Demo ===\n\n");

    MM1 q1 = {8.0, 10.0};
    printf("M/M/1 Queue: lambda=%.0f, mu=%.0f\n", q1.lambda, q1.mu);
    printf("  Utilization (rho):        %.4f\n", mm1_utilization(&q1));
    printf("  Avg queue length (L):     %.4f\n", mm1_avg_queue_length(&q1));
    printf("  Avg wait time (W):        %.4f\n", mm1_avg_wait_time(&q1));
    printf("  P0 (empty):               %.4f\n", mm1_prob_n(&q1, 0));
    printf("  P1 (1 in system):         %.4f\n", mm1_prob_n(&q1, 1));
    printf("  P5 (5 in system):         %.4f\n\n", mm1_prob_n(&q1, 5));

    MMc q2 = {8.0, 10.0, 2};
    printf("M/M/2 Queue: lambda=%.0f, mu=%.0f, c=%d\n", q2.lambda, q2.mu, q2.c);
    printf("  Utilization (rho):        %.4f\n", mmc_utilization(&q2));
    printf("  P0 (empty):               %.6f\n", mmc_prob_zero(&q2));
    printf("  Avg queue length (Lq):    %.6f\n", mmc_avg_queue_length(&q2));
    printf("  Avg wait time (Wq):       %.6f\n\n", mmc_avg_wait_time(&q2));

    printf("G/G/1 Simulation (1000 customers):\n");
    int n_customers = 1000;
    double* arrivals = (double*)malloc(n_customers * sizeof(double));
    double* services = (double*)malloc(n_customers * sizeof(double));
    double* wait_times = (double*)malloc(n_customers * sizeof(double));

    double t = 0;
    for (int i = 0; i < n_customers; i++) {
        arrivals[i] = t;
        t += exponential_rand(q1.lambda);
        services[i] = exponential_rand(q1.mu);
    }

    queue_simulate_g_g_1(arrivals, services, n_customers, wait_times);
    queue_print_stats(wait_times, n_customers);

    double avg_wait = 0;
    for (int i = 0; i < n_customers; i++) avg_wait += wait_times[i];
    avg_wait /= n_customers;
    double avg_lambda = n_customers / t;
    double L = littles_law_L(avg_lambda, avg_wait);
    double W = littles_law_W(L, avg_lambda);
    printf("\nLittle's Law Verification:\n");
    printf("  Empirical lambda = %.4f\n", avg_lambda);
    printf("  Empirical W = %.6f\n", avg_wait);
    printf("  L = lambda * W = %.4f\n", L);
    printf("  W = L / lambda = %.6f (match)\n\n", W);

    printf("Theoretical M/M/1: L=%.4f, W=%.4f\n",
           mm1_avg_queue_length(&q1), mm1_avg_wait_time(&q1));

    free(arrivals);
    free(services);
    free(wait_times);
    printf("\nDone.\n");
    return 0;
}
