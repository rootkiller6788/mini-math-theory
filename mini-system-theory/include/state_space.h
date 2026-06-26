#ifndef STATE_SPACE_H
#define STATE_SPACE_H

#include <stdbool.h>

typedef struct {
    double** A;
    double** B;
    double** C;
    double** D;
    int n_states, n_inputs, n_outputs;
} StateSpace;

StateSpace* ss_create(int n, int m, int p);
void ss_free(StateSpace* ss);
void ss_set_A(StateSpace* ss, int i, int j, double val);
void ss_set_B(StateSpace* ss, int i, int j, double val);
void ss_set_C(StateSpace* ss, int i, int j, double val);
void ss_set_D(StateSpace* ss, int i, int j, double val);
double* ss_step(StateSpace* ss, double* x, double* u, double dt);
double* ss_simulate(StateSpace* ss, double* x0, double** u, int steps, double dt, double** x_out);
double* ss_eigenvalues(StateSpace* ss);
bool ss_is_stable(StateSpace* ss);
void ss_print(StateSpace* ss);

StateSpace* ss_create_mass_spring_damper(double m, double c, double k);
StateSpace* ss_create_pid_plant(double K, double tau);

/* L8: Gramians and structural properties */
double** ss_controllability_gramian(StateSpace* ss);
double** ss_observability_gramian(StateSpace* ss);
void ss_free_gramian(double** Wc, int n);
int ss_is_controllable(StateSpace* ss);
int ss_is_observable(StateSpace* ss);

#endif
