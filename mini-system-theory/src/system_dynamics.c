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
