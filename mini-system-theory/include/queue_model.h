#ifndef QUEUE_MODEL_H
#define QUEUE_MODEL_H

typedef struct {
    double lambda, mu;
} MM1;

typedef struct {
    double lambda, mu;
    int c;
} MMc;

typedef struct {
    double* arrivals;
    double* service_times;
    int n;
} QueueTrace;

double mm1_utilization(MM1* q);
double mm1_avg_queue_length(MM1* q);
double mm1_avg_wait_time(MM1* q);
double mm1_prob_n(MM1* q, int n);

double mmc_utilization(MMc* q);
double mmc_prob_zero(MMc* q);
double mmc_avg_queue_length(MMc* q);
double mmc_avg_wait_time(MMc* q);

double* queue_simulate_g_g_1(double* arrivals, double* service, int n, double* wait_times);
void queue_print_stats(double* wait_times, int n);

double littles_law_L(double lambda, double W);
double littles_law_W(double L, double lambda);

/* L7: Priority queue (non-preemptive) */
typedef struct {
    double lambda1, lambda2;
    double mu;
    double rho1, rho2;
} PriorityQueue;

void pq_init(PriorityQueue* pq, double lambda1, double lambda2, double mu);
double pq_rho_total(PriorityQueue* pq);
double pq_avg_wait_type1(PriorityQueue* pq);
double pq_avg_wait_type2(PriorityQueue* pq);

/* L7: Jackson queueing network */
typedef struct {
    double* lambda_ext;
    double** routing;
    double* mu;
    int* servers;
    double* total_lambda;
    int n_nodes;
} JacksonNetwork;

JacksonNetwork* jn_create(int n_nodes);
void jn_free(JacksonNetwork* jn);
void jn_solve_traffic(JacksonNetwork* jn, int max_iter, double tol);
void jn_node_metrics(JacksonNetwork* jn, double* L_per_node, double* W_per_node);
double jn_total_L(JacksonNetwork* jn);

/* L5: M/G/1 queue (Pollaczek-Khinchine) */
double mg1_avg_queue_length(double lambda, double mu, double cv_sq);
double mg1_avg_wait_time(double lambda, double mu, double cv_sq);

#endif
