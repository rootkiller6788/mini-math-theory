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

#endif
