#ifndef NONLINEAR_H
#define NONLINEAR_H

#include <stdbool.h>

/* L1: Nonlinear system represented as xdot = f(x, u, t) */
typedef struct {
    /* Function pointer: computes dx/dt given state x, input u, time t */
    void (*dynamics)(double* x, double* u, double t, double* dxdt);
    int n_states;
    int n_inputs;
} NonlinearSystem;

/* L1: Equilibrium point of a nonlinear system */
typedef struct {
    double* x_eq;
    double* u_eq;
    int n_states;
} EquilibriumPoint;

/* L5: RK4 integrator for nonlinear ODEs
 * k1 = h*f(x_n, t_n)
 * k2 = h*f(x_n + k1/2, t_n + h/2)
 * k3 = h*f(x_n + k2/2, t_n + h/2)
 * k4 = h*f(x_n + k3, t_n + h)
 * x_{n+1} = x_n + (k1 + 2k2 + 2k3 + k4)/6
 * Complexity: O(n) per step, global error O(h^4)
 * Reference: Runge (1895), Kutta (1901)
 */
void rk4_step(void (*f)(double*, double*, double, double*),
              double* x, double* u, double t, double h,
              int n_states, int n_inputs, double* x_next);

void rk4_simulate(void (*f)(double*, double*, double, double*),
                  double* x0, double** u_seq, double t0, double h, int steps,
                  int n_states, int n_inputs, double** x_out);

/* L4: Linearization — compute Jacobians A = df/dx, B = df/du at equilibrium
 * Uses central finite differences: df_i/dx_j ≈ (f_i(x+εe_j) - f_i(x-εe_j)) / (2ε)
 * Reference: Khalil, H.K. (2002) Nonlinear Systems, 3rd Ed.
 */
void linearize_at_eq(NonlinearSystem* sys, EquilibriumPoint* eq,
                     double*** A_out, double*** B_out);

/* L6: Lyapunov's indirect method — stability from linearization
 * If linearization is asymptotically stable, equilibrium is asymptotically stable.
 * If linearization has eigenvalue with positive real part, equilibrium is unstable.
 */
int lyapunov_indirect(double** A, int n, double* eigenvalues_out);

/* L8: Describing function analysis for limit cycle prediction
 * Describing function N(A, ω) approximates nonlinear element gain
 * Limit cycle condition: G(jω) = -1/N(A)
 */
typedef struct {
    double amplitude;
    double frequency;
    bool stable;
} LimitCycle;

/* Saturation describing function: N(A) = (2K/π)[asin(a/A) + (a/A)sqrt(1-(a/A)^2)]
 * Valid for A > a (amplitude > saturation threshold)
 */
double describing_function_saturation(double A, double slope, double limit);

/* Relay describing function: N(A) = 4M/(πA)
 * Characteristic of ideal relay / on-off controller
 */
double describing_function_relay(double A, double M);

/* Dead-zone describing function: N(A) = K - (2K/π)[asin(δ/A) + (δ/A)sqrt(1-(δ/A)^2)]
 * Valid for A > δ
 */
double describing_function_deadzone(double A, double slope, double deadzone);

/* L2: Find limit cycles via describing function + Nyquist intersection */
LimitCycle find_limit_cycle_describing(double (*N)(double), double** A,
                                        double** B, double** C, int n);

/* L7: Van der Pol oscillator — canonical nonlinear system
 * dx/dt = y
 * dy/dt = μ(1-x²)y - x
 * Exhibits stable limit cycle for μ > 0
 * Reference: Van der Pol (1927) "On Relaxation Oscillations"
 */
void vdp_dynamics(double* x, double* u, double t, double* dxdt);
EquilibriumPoint* vdp_equilibrium(void);

/* L7: Duffing oscillator — nonlinear hardening/softening spring
 * dx/dt = y
 * dy/dt = -δy - αx - βx³ + γcos(ωt)
 */
void duffing_dynamics(double* x, double* u, double t, double* dxdt);

/* Utility: numerical Jacobian via finite differences */
void numerical_jacobian(void (*f)(double*, double*, double, double*),
                        double* x, double* u, double eps,
                        double** J_out, int n, int m);

#endif
