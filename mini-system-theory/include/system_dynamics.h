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

#endif
