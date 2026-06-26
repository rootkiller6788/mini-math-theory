/* nonlinear.c - Nonlinear Systems, Describing Functions, Linearization
 *
 * Knowledge coverage:
 * L1: NonlinearSystem, EquilibriumPoint, LimitCycle structs
 * L2: Nonlinear dynamics, equilibrium points, limit cycles
 * L3: RK4 integration, numerical Jacobian
 * L4: Lyapunov's indirect method (linearization stability)
 * L5: Describing function analysis (saturation, relay, deadzone)
 * L6: Van der Pol and Duffing oscillators
 * L7: Limit cycle prediction in feedback systems
 * L8: Describing function is an advanced (quasi-linearization) technique
 * Course alignment: MIT 6.241J Ch. 7, MIT 2.151, ETH 227-0216
 */

#include "nonlinear.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846

/* ---------- L5: RK4 Integration ---------- */

void rk4_step(void (*f)(double*, double*, double, double*),
              double* x, double* u, double t, double h,
              int n_states, int n_inputs, double* x_next) {
    double* k1 = (double*)malloc(n_states * sizeof(double));
    double* k2 = (double*)malloc(n_states * sizeof(double));
    double* k3 = (double*)malloc(n_states * sizeof(double));
    double* k4 = (double*)malloc(n_states * sizeof(double));
    double* xtemp = (double*)malloc(n_states * sizeof(double));

    /* k1 = h * f(x, u, t) */
    f(x, u, t, k1);
    for (int i = 0; i < n_states; i++) k1[i] *= h;

    /* k2 = h * f(x + k1/2, u, t + h/2) */
    for (int i = 0; i < n_states; i++) xtemp[i] = x[i] + 0.5 * k1[i];
    f(xtemp, u, t + 0.5 * h, k2);
    for (int i = 0; i < n_states; i++) k2[i] *= h;

    /* k3 = h * f(x + k2/2, u, t + h/2) */
    for (int i = 0; i < n_states; i++) xtemp[i] = x[i] + 0.5 * k2[i];
    f(xtemp, u, t + 0.5 * h, k3);
    for (int i = 0; i < n_states; i++) k3[i] *= h;

    /* k4 = h * f(x + k3, u, t + h) */
    for (int i = 0; i < n_states; i++) xtemp[i] = x[i] + k3[i];
    f(xtemp, u, t + h, k4);
    for (int i = 0; i < n_states; i++) k4[i] *= h;

    /* x_next = x + (k1 + 2*k2 + 2*k3 + k4) / 6 */
    for (int i = 0; i < n_states; i++)
        x_next[i] = x[i] + (k1[i] + 2.0*k2[i] + 2.0*k3[i] + k4[i]) / 6.0;

    free(k1); free(k2); free(k3); free(k4); free(xtemp);
}

void rk4_simulate(void (*f)(double*, double*, double, double*),
                  double* x0, double** u_seq, double t0, double h, int steps,
                  int n_states, int n_inputs, double** x_out) {
    if (x_out) {
        *x_out = (double*)malloc((steps + 1) * n_states * sizeof(double));
        for (int i = 0; i < n_states; i++)
            (*x_out)[i] = x0[i];
    }

    double* x = (double*)malloc(n_states * sizeof(double));
    double* x_next = (double*)malloc(n_states * sizeof(double));
    memcpy(x, x0, n_states * sizeof(double));

    for (int k = 0; k < steps; k++) {
        double* u = u_seq ? u_seq[k] : NULL;
        rk4_step(f, x, u, t0 + k * h, h, n_states, n_inputs, x_next);
        memcpy(x, x_next, n_states * sizeof(double));
        if (x_out)
            for (int i = 0; i < n_states; i++)
                (*x_out)[(k + 1) * n_states + i] = x[i];
    }

    free(x); free(x_next);
}

/* ---------- L4: Numerical Jacobian via Central Differences ---------- */

void numerical_jacobian(void (*f)(double*, double*, double, double*),
                        double* x, double* u, double eps,
                        double** J_out, int n, int m) {
    double* dx_plus = (double*)malloc(n * sizeof(double));
    double* dx_minus = (double*)malloc(n * sizeof(double));
    double* x_pert = (double*)malloc(n * sizeof(double));

    for (int j = 0; j < n + m; j++) {
        memcpy(x_pert, x, n * sizeof(double));

        if (j < n) {
            /* Perturb state x_j */
            double orig = x_pert[j];
            x_pert[j] = orig + eps;
            f(x_pert, u, 0.0, dx_plus);
            x_pert[j] = orig - eps;
            f(x_pert, u, 0.0, dx_minus);
            for (int i = 0; i < n; i++)
                J_out[i][j] = (dx_plus[i] - dx_minus[i]) / (2.0 * eps);
            x_pert[j] = orig;
        } else {
            /* Perturb input u_{j-n} */
            double orig = u[j - n];
            u[j - n] = orig + eps;
            f(x, u, 0.0, dx_plus);
            u[j - n] = orig - eps;
            f(x, u, 0.0, dx_minus);
            for (int i = 0; i < n; i++)
                J_out[i][j] = (dx_plus[i] - dx_minus[i]) / (2.0 * eps);
            u[j - n] = orig;
        }
    }

    free(dx_plus); free(dx_minus); free(x_pert);
}

/* ---------- L4: Linearization at Equilibrium ---------- */

void linearize_at_eq(NonlinearSystem* sys, EquilibriumPoint* eq,
                     double*** A_out, double*** B_out) {
    int n = sys->n_states, m = sys->n_inputs;

    *A_out = (double**)malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++)
        (*A_out)[i] = (double*)calloc(n, sizeof(double));

    *B_out = (double**)malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++)
        (*B_out)[i] = (double*)calloc(m, sizeof(double));

    double* temp = (double*)malloc(n * sizeof(double));
    double eps = 1e-6;

    /* A = df/dx at equilibrium */
    for (int j = 0; j < n; j++) {
        double* xp = (double*)malloc(n * sizeof(double));
        double* xn = (double*)malloc(n * sizeof(double));
        double* fp = (double*)malloc(n * sizeof(double));
        double* fn = (double*)malloc(n * sizeof(double));

        memcpy(xp, eq->x_eq, n * sizeof(double));
        memcpy(xn, eq->x_eq, n * sizeof(double));
        xp[j] += eps;
        xn[j] -= eps;

        sys->dynamics(xp, eq->u_eq, 0.0, fp);
        sys->dynamics(xn, eq->u_eq, 0.0, fn);

        for (int i = 0; i < n; i++)
            (*A_out)[i][j] = (fp[i] - fn[i]) / (2.0 * eps);

        free(xp); free(xn); free(fp); free(fn);
    }

    /* B = df/du at equilibrium */
    for (int j = 0; j < m; j++) {
        double* up = (double*)malloc(m * sizeof(double));
        double* un = (double*)malloc(m * sizeof(double));
        double* fp = (double*)malloc(n * sizeof(double));
        double* fn = (double*)malloc(n * sizeof(double));

        memcpy(up, eq->u_eq, m * sizeof(double));
        memcpy(un, eq->u_eq, m * sizeof(double));
        up[j] += eps;
        un[j] -= eps;

        sys->dynamics(eq->x_eq, up, 0.0, fp);
        sys->dynamics(eq->x_eq, un, 0.0, fn);

        for (int i = 0; i < n; i++)
            (*B_out)[i][j] = (fp[i] - fn[i]) / (2.0 * eps);

        free(up); free(un); free(fp); free(fn);
    }

    free(temp);
}

/* ---------- L6: Lyapunov's Indirect Method ---------- */
int lyapunov_indirect(double** A, int n, double* eigenvalues_out) {
    /* Compute trace and determinant for 2x2; general eigenvalue extraction
     * For n=2: eigenvalues satisfy lambda^2 - trace*lambda + det = 0 */
    if (n == 2) {
        double trace = A[0][0] + A[1][1];
        double det = A[0][0] * A[1][1] - A[0][1] * A[1][0];
        double disc = trace * trace - 4.0 * det;

        if (disc >= 0) {
            eigenvalues_out[0] = (trace + sqrt(disc)) / 2.0;
            eigenvalues_out[1] = 0.0;
            eigenvalues_out[2] = (trace - sqrt(disc)) / 2.0;
            eigenvalues_out[3] = 0.0;
        } else {
            eigenvalues_out[0] = trace / 2.0;
            eigenvalues_out[1] = sqrt(-disc) / 2.0;
            eigenvalues_out[2] = trace / 2.0;
            eigenvalues_out[3] = -sqrt(-disc) / 2.0;
        }
        if (eigenvalues_out[0] < 0 && eigenvalues_out[2] < 0) return 1;   /* stable */
        if (eigenvalues_out[0] > 0 || eigenvalues_out[2] > 0) return -1; /* unstable */
        return 0; /* marginally stable */
    }

    /* For n > 2, approximate via power iteration for dominant eigenvalue */
    double* v = (double*)malloc(n * sizeof(double));
    double* Av = (double*)malloc(n * sizeof(double));
    for (int i = 0; i < n; i++) v[i] = 1.0;
    double lambda = 0.0;

    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < n; i++) {
            Av[i] = 0.0;
            for (int j = 0; j < n; j++) Av[i] += A[i][j] * v[j];
        }
        double norm = 0.0;
        for (int i = 0; i < n; i++) norm += Av[i] * Av[i];
        norm = sqrt(norm);
        if (norm < 1e-12) break;
        double new_lambda = 0.0;
        for (int i = 0; i < n; i++) {
            v[i] = Av[i] / norm;
            new_lambda += v[i] * Av[i];
        }
        if (fabs(new_lambda - lambda) < 1e-8) { lambda = new_lambda; break; }
        lambda = new_lambda;
    }

    eigenvalues_out[0] = lambda;
    eigenvalues_out[1] = 0.0;

    free(v); free(Av);
    if (lambda < 0) return 1;
    if (lambda > 0) return -1;
    return 0;
}

/* ---------- L8: Describing Functions ---------- */

double describing_function_saturation(double A, double slope, double limit) {
    if (A <= limit) return slope;
    double a_over_A = limit / A;
    double term = a_over_A * sqrt(1.0 - a_over_A * a_over_A);
    return (2.0 * slope / PI) * (asin(a_over_A) + term);
}

double describing_function_relay(double A, double M) {
    if (A < 1e-12) return INFINITY;
    return 4.0 * M / (PI * A);
}

double describing_function_deadzone(double A, double slope, double deadzone) {
    if (A <= deadzone) return 0.0;
    double d_over_A = deadzone / A;
    double term = d_over_A * sqrt(1.0 - d_over_A * d_over_A);
    return slope - (2.0 * slope / PI) * (asin(d_over_A) + term);
}

/* ---------- L2: Limit Cycle Detection via Describing Function ---------- */

LimitCycle find_limit_cycle_describing(double (*N)(double), double** A,
                                        double** B, double** C, int n) {
    LimitCycle lc = {0.0, 0.0, false};

    /* Scan amplitudes A to find where G(jw) = -1/N(A)
     * For first-order plant 1/(s+tau): |G(jw)| = 1/sqrt(w^2+tau^2)
     * For this demo, assume scalar system with known transfer function
     */
    double tau = -1.0 / A[0][0]; /* First-order: dx/dt = -x/tau + u/tau */
    if (tau <= 0) return lc;

    /* Scan frequencies w, find where magnitude condition holds */
    for (int i = 1; i < 1000; i++) {
        double w = 0.01 * i;
        double G_mag = B[0][0] / tau / sqrt(w * w + 1.0 / (tau * tau));
        double G_phase = atan2(-w, 1.0 / tau); /* Phase of 1/(j*w*tau+1) */

        /* Need G_phase = -PI (odd) for real -1/N(A) */
        /* Actually we solve |G(jw)| * N(A) = 1 and phase = -PI */
        for (int j = 1; j < 200; j++) {
            double A_val = 0.01 * j;
            double N_val = N(A_val);
            if (N_val < 1e-12) continue;

            if (fabs(G_mag * N_val - 1.0) < 0.05 &&
                fabs(G_phase + PI) < 0.1) {
                lc.amplitude = A_val;
                lc.frequency = w;
                lc.stable = true; /* heuristic for relay */
                return lc;
            }
        }
    }
    return lc;
}

/* ---------- L7: Van der Pol Oscillator ---------- */

void vdp_dynamics(double* x, double* u, double t, double* dxdt) {
    double mu = (u && fabs(u[0]) > 1e-12) ? u[0] : 1.0;
    (void)t;
    dxdt[0] = x[1];
    dxdt[1] = mu * (1.0 - x[0]*x[0]) * x[1] - x[0];
}

EquilibriumPoint* vdp_equilibrium(void) {
    EquilibriumPoint* eq = (EquilibriumPoint*)malloc(sizeof(EquilibriumPoint));
    eq->n_states = 2;
    eq->x_eq = (double*)calloc(2, sizeof(double));
    eq->u_eq = (double*)calloc(1, sizeof(double));
    eq->u_eq[0] = 1.0;  /* mu = 1 */
    eq->x_eq[0] = 0.0;
    eq->x_eq[1] = 0.0;
    return eq;
}

/* ---------- L7: Duffing Oscillator ---------- */

void duffing_dynamics(double* x, double* u, double t, double* dxdt) {
    double delta = (u && fabs(u[0]) > 1e-12) ? u[0] : 0.1;
    double alpha = (u && fabs(u[1]) > 1e-12) ? u[1] : -1.0;
    double beta  = (u && fabs(u[2]) > 1e-12) ? u[2] : 1.0;
    double gamma = (u && fabs(u[3]) > 1e-12) ? u[3] : 0.3;
    double omega = (u && fabs(u[4]) > 1e-12) ? u[4] : 1.2;
    (void)t;
    dxdt[0] = x[1];
    dxdt[1] = -delta * x[1] - alpha * x[0] - beta * x[0]*x[0]*x[0] + gamma * cos(omega * t);
}
