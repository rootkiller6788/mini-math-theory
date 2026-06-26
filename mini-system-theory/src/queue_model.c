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

/* ---------- L7: Priority Queue ----------
 * Non-preemptive priority queue: type-1 (high priority) and type-2 (low priority)
 * Reference: Kleinrock (1976) Queueing Systems Vol. 2
 * (PriorityQueue struct defined in queue_model.h)
 */
void pq_init(PriorityQueue* pq, double lambda1, double lambda2, double mu) {
    pq->lambda1 = lambda1;
    pq->lambda2 = lambda2;
    pq->mu = mu;
    pq->rho1 = lambda1 / mu;
    pq->rho2 = lambda2 / mu;
}

double pq_rho_total(PriorityQueue* pq) {
    return (pq->lambda1 + pq->lambda2) / pq->mu;
}

double pq_avg_wait_type1(PriorityQueue* pq) {
    double rho_total = pq_rho_total(pq);
    if (rho_total >= 1.0) return INFINITY;
    return (pq->rho1 + pq->rho2) / (pq->mu * (1.0 - pq->rho1));
}

double pq_avg_wait_type2(PriorityQueue* pq) {
    double rho_total = pq_rho_total(pq);
    if (rho_total >= 1.0) return INFINITY;
    return (pq->rho1 + pq->rho2) /
           (pq->mu * (1.0 - pq->rho1) * (1.0 - rho_total));
}

/* ---------- L7: Jackson Queueing Network ----------
 * Open queueing network with M nodes
 * Each node i: M/M/c_i with external arrival rate gamma_i
 * Routing probabilities r_ij (from node i to node j)
 * Reference: Jackson, J.R. (1957) "Networks of Waiting Lines"
 * (JacksonNetwork struct defined in queue_model.h)
 */
JacksonNetwork* jn_create(int n_nodes) {
    JacksonNetwork* jn = (JacksonNetwork*)malloc(sizeof(JacksonNetwork));
    jn->n_nodes = n_nodes;
    jn->lambda_ext = (double*)calloc(n_nodes, sizeof(double));
    jn->mu = (double*)calloc(n_nodes, sizeof(double));
    jn->servers = (int*)calloc(n_nodes, sizeof(int));
    jn->total_lambda = (double*)calloc(n_nodes, sizeof(double));
    jn->routing = (double**)malloc(n_nodes * sizeof(double*));
    for (int i = 0; i < n_nodes; i++) {
        jn->routing[i] = (double*)calloc(n_nodes, sizeof(double));
        jn->servers[i] = 1;
    }
    return jn;
}

void jn_free(JacksonNetwork* jn) {
    free(jn->lambda_ext); free(jn->mu); free(jn->servers); free(jn->total_lambda);
    for (int i = 0; i < jn->n_nodes; i++) free(jn->routing[i]);
    free(jn->routing); free(jn);
}

/* Solve traffic equations: lambda_total = lambda_ext + sum_j lambda_total[j] * r_ji
 * Using Gauss-Seidel iteration */
void jn_solve_traffic(JacksonNetwork* jn, int max_iter, double tol) {
    int n = jn->n_nodes;
    for (int i = 0; i < n; i++) jn->total_lambda[i] = jn->lambda_ext[i];

    for (int iter = 0; iter < max_iter; iter++) {
        double max_diff = 0.0;
        for (int i = 0; i < n; i++) {
            double old = jn->total_lambda[i];
            double inflow = jn->lambda_ext[i];
            for (int j = 0; j < n; j++)
                inflow += jn->total_lambda[j] * jn->routing[j][i];
            jn->total_lambda[i] = inflow;
            double diff = fabs(jn->total_lambda[i] - old);
            if (diff > max_diff) max_diff = diff;
        }
        if (max_diff < tol) break;
    }
}

/* Compute average number of jobs at each node (M/M/c) */
void jn_node_metrics(JacksonNetwork* jn, double* L_per_node, double* W_per_node) {
    for (int i = 0; i < jn->n_nodes; i++) {
        double lam = jn->total_lambda[i];
        double mu = jn->mu[i];
        int c = jn->servers[i];
        double rho = lam / (mu * c);

        if (rho >= 1.0) {
            L_per_node[i] = INFINITY;
            W_per_node[i] = INFINITY;
            continue;
        }

        /* Compute P0 */
        double r = lam / mu;
        double sum = 0.0;
        double fact = 1.0;
        for (int n = 0; n < c; n++) {
            sum += pow(r, n) / fact;
            fact *= (n + 1);
        }
        fact = 1.0;
        for (int n = 1; n <= c; n++) fact *= n;
        double term = pow(r, c) / (fact * (1.0 - rho));
        double P0 = 1.0 / (sum + term);

        /* Lq */
        double Lq = P0 * pow(r, c) * rho / (fact * (1.0 - rho) * (1.0 - rho));
        L_per_node[i] = Lq + r;
        W_per_node[i] = L_per_node[i] / lam;
    }
}

double jn_total_L(JacksonNetwork* jn) {
    double* L = (double*)malloc(jn->n_nodes * sizeof(double));
    double* W = (double*)malloc(jn->n_nodes * sizeof(double));
    jn_node_metrics(jn, L, W);
    double total = 0.0;
    for (int i = 0; i < jn->n_nodes; i++) total += L[i];
    free(L); free(W);
    return total;
}

/* ---------- L5: M/G/1 Queue (Pollaczek-Khinchine formula) ----------
 * L = rho + rho^2*(1 + C_s^2) / (2*(1-rho))
 * where C_s = sigma_s / E[S] is the service time coefficient of variation
 */
double mg1_avg_queue_length(double lambda, double mu, double cv_sq) {
    double rho = lambda / mu;
    if (rho >= 1.0) return INFINITY;
    return rho + (rho * rho * (1.0 + cv_sq)) / (2.0 * (1.0 - rho));
}

double mg1_avg_wait_time(double lambda, double mu, double cv_sq) {
    double rho = lambda / mu;
    if (rho >= 1.0) return INFINITY;
    return (rho * (1.0 + cv_sq)) / (2.0 * mu * (1.0 - rho));
}
