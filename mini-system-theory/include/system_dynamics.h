#ifndef SYSTEM_DYNAMICS_H
#define SYSTEM_DYNAMICS_H

typedef struct {
    double level;
    double* inflows;
    double* outflows;
    int n_flows;
} Stock;

typedef struct {
    double x, goal, time_constant;
} FirstOrder;

typedef struct {
    double x, v, stiffness, damping;
} SecondOrder;

void stock_init(Stock* s, double initial, int n_flows);
double stock_update(Stock* s, double dt);

void fo_init(FirstOrder* fo, double initial, double goal, double tc);
double fo_update(FirstOrder* fo, double dt);

void so_init(SecondOrder* so, double x0, double v0, double k, double c);
void so_update(SecondOrder* so, double dt, double* position, double* velocity);

double exp_smooth_update(double* smooth, double alpha, double new_value);

/* L6: Pipeline delay (nth-order Erlang) */
typedef struct {
    double* delay_stages;
    int order;
    double delay_time;
    double dt;
} PipelineDelay;

void pd_init(PipelineDelay* pd, int order, double delay_time, double dt);
void pd_free(PipelineDelay* pd);
double pd_update(PipelineDelay* pd, double inflow);

/* L6: SIR epidemic model */
typedef struct {
    double S, I, R;
    double beta, gamma;
    double N;
} SIRModel;

void sir_init(SIRModel* sir, double population, double initial_infected,
              double beta, double gamma);
void sir_step(SIRModel* sir, double dt);
void sir_get_state(SIRModel* sir, double* S, double* I, double* R);

/* L6: Aging chain / cohort model */
typedef struct {
    double* cohorts;
    double* death_rates;
    double avg_lifespan;
    int n_cohorts;
} AgingChain;

AgingChain* ac_create(int n_cohorts, double total_lifespan);
void ac_free(AgingChain* ac);
void ac_update(AgingChain* ac, double dt, double birth_rate, double total_population);
double ac_total_population(AgingChain* ac);
void ac_print_distribution(AgingChain* ac);

/* L3: RK2 (midpoint) integration for system dynamics */
typedef void (*SDEquations)(double* stocks, double* aux, int n, double t, double* flows);
void sd_rk2_integrate(double* stocks, int n_stocks, SDEquations eqs,
                      double* aux, double t, double dt);

#endif
