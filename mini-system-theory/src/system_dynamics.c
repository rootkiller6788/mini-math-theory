#include "system_dynamics.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PI 3.14159265358979323846

void stock_init(Stock* s, double initial, int n_flows) {
    s->level = initial;
    s->n_flows = n_flows;
    s->inflows = (double*)calloc(n_flows, sizeof(double));
    s->outflows = (double*)calloc(n_flows, sizeof(double));
}

double stock_update(Stock* s, double dt) {
    double net = 0;
    for (int i = 0; i < s->n_flows; i++)
        net += s->inflows[i] - s->outflows[i];
    s->level += net * dt;
    return s->level;
}

void fo_init(FirstOrder* fo, double initial, double goal, double tc) {
    fo->x = initial;
    fo->goal = goal;
    fo->time_constant = tc;
}

double fo_update(FirstOrder* fo, double dt) {
    double dx = (fo->goal - fo->x) / fo->time_constant;
    fo->x += dx * dt;
    return fo->x;
}

void so_init(SecondOrder* so, double x0, double v0, double k, double c) {
    so->x = x0;
    so->v = v0;
    so->stiffness = k;
    so->damping = c;
}

void so_update(SecondOrder* so, double dt, double* position, double* velocity) {
    double a = -so->damping * so->v - so->stiffness * so->x;
    so->v += a * dt;
    so->x += so->v * dt;
    *position = so->x;
    *velocity = so->v;
}

double exp_smooth_update(double* smooth, double alpha, double new_value) {
    *smooth = alpha * new_value + (1.0 - alpha) * (*smooth);
    return *smooth;
}

/* ---------- L6: Pipeline Delay (material/information delay) ----------
 * nth-order exponential delay (Erlang distribution)
 * Models the time lag between cause and effect in system dynamics.
 * Reference: Sterman (2000) Business Dynamics
 * (PipelineDelay struct defined in system_dynamics.h)
 */
void pd_init(PipelineDelay* pd, int order, double delay_time, double dt) {
    pd->order = order;
    pd->delay_time = delay_time;
    pd->dt = dt;
    pd->delay_stages = (double*)calloc(order, sizeof(double));
}

void pd_free(PipelineDelay* pd) {
    free(pd->delay_stages);
}

double pd_update(PipelineDelay* pd, double inflow) {
    int n = pd->order;
    double tau = pd->delay_time / n;
    double dt = pd->dt;

    /* Outflow from last stage */
    double outflow = pd->delay_stages[n - 1] / tau;

    /* Shift stages: each stage decays at rate 1/tau */
    for (int i = n - 1; i > 0; i--) {
        pd->delay_stages[i] += dt * (pd->delay_stages[i - 1] - pd->delay_stages[i]) / tau;
    }
    pd->delay_stages[0] += dt * (inflow - pd->delay_stages[0]) / tau;

    return outflow;
}

/* ---------- L3: Third-order system for realistic dynamics ---------- */
typedef struct {
    double x, y, z;
    double a, b, c;
} ThirdOrder;

void to_init(ThirdOrder* to, double x0, double a, double b, double c) {
    to->x = x0;
    to->y = 0.0;
    to->z = 0.0;
    to->a = a;
    to->b = b;
    to->c = c;
}

void to_update(ThirdOrder* to, double dt, double input) {
    /* dx/dt = y, dy/dt = z, dz/dt = -a*z - b*y - c*x + input */
    double dx = to->y;
    double dy = to->z;
    double dz = -to->a * to->z - to->b * to->y - to->c * to->x + input;
    to->x += dx * dt;
    to->y += dy * dt;
    to->z += dz * dt;
}

double to_get_position(ThirdOrder* to) { return to->x; }
double to_get_velocity(ThirdOrder* to) { return to->y; }
double to_get_acceleration(ThirdOrder* to) { return to->z; }

/* ---------- L6: Aging Chain / Cohort Model ----------
 * Systems dynamics aging chain: population moves through age cohorts
 * Each cohort has inflow (aging in) and outflow (aging out + deaths)
 * Used for population dynamics, product lifecycle, workforce planning
 * (AgingChain struct defined in system_dynamics.h)
 */
AgingChain* ac_create(int n_cohorts, double total_lifespan) {
    AgingChain* ac = (AgingChain*)malloc(sizeof(AgingChain));
    ac->n_cohorts = n_cohorts;
    ac->avg_lifespan = total_lifespan;
    ac->cohorts = (double*)calloc(n_cohorts, sizeof(double));
    ac->death_rates = (double*)malloc(n_cohorts * sizeof(double));
    double cohort_lifespan = total_lifespan / n_cohorts;
    for (int i = 0; i < n_cohorts; i++)
        ac->death_rates[i] = (i == n_cohorts - 1) ? 1.0 / cohort_lifespan : 0.0;
    return ac;
}

void ac_free(AgingChain* ac) {
    free(ac->cohorts);
    free(ac->death_rates);
    free(ac);
}

void ac_update(AgingChain* ac, double dt, double birth_rate, double total_population) {
    int n = ac->n_cohorts;
    double aging_rate = n / ac->avg_lifespan;

    /* Births enter first cohort */
    double births = birth_rate * total_population;
    ac->cohorts[0] += births * dt;

    /* Aging flows */
    double* new_cohorts = (double*)malloc(n * sizeof(double));
    for (int i = 0; i < n; i++) {
        double inflow = (i > 0) ? ac->cohorts[i - 1] * aging_rate * dt : 0.0;
        double outflow = ac->cohorts[i] * aging_rate * dt;
        double deaths = ac->cohorts[i] * ac->death_rates[i] * dt;
        new_cohorts[i] = ac->cohorts[i] + inflow - outflow - deaths;
        if (new_cohorts[i] < 0) new_cohorts[i] = 0;
    }

    for (int i = 0; i < n; i++) ac->cohorts[i] = new_cohorts[i];
    free(new_cohorts);
}

double ac_total_population(AgingChain* ac) {
    double total = 0.0;
    for (int i = 0; i < ac->n_cohorts; i++)
        total += ac->cohorts[i];
    return total;
}

void ac_print_distribution(AgingChain* ac) {
    printf("Age Cohort Distribution (%d cohorts):\n", ac->n_cohorts);
    for (int i = 0; i < ac->n_cohorts; i++) {
        double age_start = i * ac->avg_lifespan / ac->n_cohorts;
        double age_end = (i + 1) * ac->avg_lifespan / ac->n_cohorts;
        printf("  [%5.1f - %5.1f]: %8.1f\n", age_start, age_end, ac->cohorts[i]);
    }
}

/* ---------- L3: System Dynamics Integration with Runge-Kutta 2 ----------
 * Midpoint method (RK2): more accurate than Euler for system dynamics models
 */
typedef void (*SDEquations)(double* stocks, double* aux, int n, double t, double* flows);

void sd_rk2_integrate(double* stocks, int n_stocks, SDEquations eqs,
                      double* aux, double t, double dt) {
    double* k1 = (double*)malloc(n_stocks * sizeof(double));
    double* k2 = (double*)malloc(n_stocks * sizeof(double));
    double* temp = (double*)malloc(n_stocks * sizeof(double));

    /* k1 = f(stocks, t) * dt */
    eqs(stocks, aux, n_stocks, t, k1);

    /* temp = stocks + k1 */
    for (int i = 0; i < n_stocks; i++)
        temp[i] = stocks[i] + k1[i] * dt;

    /* k2 = f(temp, t + dt) * dt */
    eqs(temp, aux, n_stocks, t + dt, k2);

    /* stocks = stocks + (k1 + k2) * dt / 2 */
    for (int i = 0; i < n_stocks; i++)
        stocks[i] += 0.5 * (k1[i] + k2[i]) * dt;

    free(k1); free(k2); free(temp);
}

/* ---------- L6: SIR Epidemic Model ----------
 * dS/dt = -beta * S * I / N
 * dI/dt = beta * S * I / N - gamma * I
 * dR/dt = gamma * I
 * Classic compartmental model in system dynamics
 * (SIRModel struct defined in system_dynamics.h)
 */
void sir_init(SIRModel* sir, double population, double initial_infected,
              double beta, double gamma) {
    sir->N = population;
    sir->S = population - initial_infected;
    sir->I = initial_infected;
    sir->R = 0.0;
    sir->beta = beta;
    sir->gamma = gamma;
}

void sir_step(SIRModel* sir, double dt) {
    double new_infections = sir->beta * sir->S * sir->I / sir->N;
    double recoveries = sir->gamma * sir->I;
    sir->S += (-new_infections) * dt;
    sir->I += (new_infections - recoveries) * dt;
    sir->R += recoveries * dt;
    if (sir->S < 0) sir->S = 0;
    if (sir->I < 0) sir->I = 0;
}

void sir_get_state(SIRModel* sir, double* S, double* I, double* R) {
    *S = sir->S; *I = sir->I; *R = sir->R;
}
