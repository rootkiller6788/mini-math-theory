#include "queue_model.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define PI 3.14159265358979323846

double mm1_utilization(MM1* q) {
    return q->lambda / q->mu;
}

double mm1_avg_queue_length(MM1* q) {
    double rho = mm1_utilization(q);
    if (rho >= 1.0) return 1e9;
    return rho / (1.0 - rho);
}

double mm1_avg_wait_time(MM1* q) {
    double rho = mm1_utilization(q);
    if (rho >= 1.0) return 1e9;
    return rho / (q->mu * (1.0 - rho));
}

double mm1_prob_n(MM1* q, int n) {
    double rho = mm1_utilization(q);
    return (1.0 - rho) * pow(rho, n);
}

static double factorial(int n) {
    double f = 1;
    for (int i = 2; i <= n; i++) f *= i;
    return f;
}

double mmc_utilization(MMc* q) {
    return q->lambda / (q->mu * q->c);
}

double mmc_prob_zero(MMc* q) {
    double rho = mmc_utilization(q);
    int c = q->c;
    double r = q->lambda / q->mu;
    double sum = 0;
    for (int n = 0; n < c; n++)
        sum += pow(r, n) / factorial(n);
    double term = pow(r, c) / (factorial(c) * (1.0 - rho));
    if (isinf(term) || isnan(term)) return 0;
    return 1.0 / (sum + term);
}

double mmc_avg_queue_length(MMc* q) {
    double rho = mmc_utilization(q);
    if (rho >= 1.0) return 1e9;
    double P0 = mmc_prob_zero(q);
    int c = q->c;
    double r = q->lambda / q->mu;
    double num = P0 * pow(r, c) * rho;
    double den = factorial(c) * pow(1.0 - rho, 2);
    return num / den;
}

double mmc_avg_wait_time(MMc* q) {
    double Lq = mmc_avg_queue_length(q);
    return Lq / q->lambda;
}

double* queue_simulate_g_g_1(double* arrivals, double* service, int n, double* wait_times) {
    double server_free = 0;
    for (int i = 0; i < n; i++) {
        double start_time = (arrivals[i] > server_free) ? arrivals[i] : server_free;
        wait_times[i] = start_time - arrivals[i];
        server_free = start_time + service[i];
    }
    return wait_times;
}

void queue_print_stats(double* wait_times, int n) {
    double sum = 0, min_w = 1e9, max_w = -1e9;
    for (int i = 0; i < n; i++) {
        sum += wait_times[i];
        if (wait_times[i] < min_w) min_w = wait_times[i];
        if (wait_times[i] > max_w) max_w = wait_times[i];
    }
    double avg = sum / n;
    double var = 0;
    for (int i = 0; i < n; i++)
        var += (wait_times[i] - avg) * (wait_times[i] - avg);
    var /= n;
    printf("Queue Statistics (n=%d):\n", n);
    printf("  Avg wait: %.6f\n", avg);
    printf("  Min wait: %.6f\n", min_w);
    printf("  Max wait: %.6f\n", max_w);
    printf("  Std dev:  %.6f\n", sqrt(var));
}

double littles_law_L(double lambda, double W) {
    return lambda * W;
}

double littles_law_W(double L, double lambda) {
    return L / lambda;
}
