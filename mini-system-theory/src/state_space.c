#include "state_space.h"
#include "stability.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PI 3.14159265358979323846

static double** mat_create(int r, int c) {
    double** m = (double**)malloc(r * sizeof(double*));
    for (int i = 0; i < r; i++) {
        m[i] = (double*)calloc(c, sizeof(double));
    }
    return m;
}

static void mat_free(double** m, int r) {
    for (int i = 0; i < r; i++) free(m[i]);
    free(m);
}

static void mat_copy(double** dst, double** src, int r, int c) {
    for (int i = 0; i < r; i++)
        for (int j = 0; j < c; j++)
            dst[i][j] = src[i][j];
}

static void mat_mul(double** C, double** A, double** B, int n, int m, int p) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < p; j++) {
            C[i][j] = 0;
            for (int k = 0; k < m; k++)
                C[i][j] += A[i][k] * B[k][j];
        }
}

static void mat_vec_mul(double* y, double** A, double* x, int r, int c) {
    for (int i = 0; i < r; i++) {
        y[i] = 0;
        for (int j = 0; j < c; j++)
            y[i] += A[i][j] * x[j];
    }
}

static double vec_norm(double* v, int n) {
    double s = 0;
    for (int i = 0; i < n; i++) s += v[i] * v[i];
    return sqrt(s);
}

static void householder_vec(double* u, double* x, int n, int start) {
    double sigma = 0;
    for (int i = start; i < n; i++) sigma += x[i] * x[i];
    double norm_x = sqrt(sigma);
    if (norm_x < 1e-15) { u[0] = 0; return; }
    double alpha = x[start];
    double beta = (alpha >= 0) ? -norm_x : norm_x;
    for (int i = 0; i < n; i++) u[i] = 0;
    u[start] = alpha - beta;
    for (int i = start + 1; i < n; i++) u[i] = x[i];
    double u_norm = sqrt(sigma - alpha * alpha + (alpha - beta) * (alpha - beta));
    if (u_norm > 1e-15)
        for (int i = start; i < n; i++) u[i] /= u_norm;
}

static void hessenberg_reduce(double** A, int n) {
    double* u = (double*)malloc(n * sizeof(double));
    double* temp = (double*)malloc(n * sizeof(double));
    for (int k = 0; k < n - 2; k++) {
        for (int i = k + 1; i < n; i++) u[i] = A[i][k];
        householder_vec(u, u, n, k + 1);
        if (u[k + 1] == 0) continue;
        for (int j = k; j < n; j++) {
            double s = 0;
            for (int i = k + 1; i < n; i++) s += u[i] * A[i][j];
            for (int i = k + 1; i < n; i++) A[i][j] -= 2 * u[i] * s;
        }
        for (int i = 0; i < n; i++) {
            double s = 0;
            for (int j = k + 1; j < n; j++) s += u[j] * A[i][j];
            for (int j = k + 1; j < n; j++) A[i][j] -= 2 * u[j] * s;
        }
    }
    free(u);
    free(temp);
}

static void givens_rotation(double a, double b, double* c, double* s) {
    if (fabs(b) < 1e-15) { *c = 1; *s = 0; return; }
    if (fabs(b) > fabs(a)) {
        double tau = -a / b;
        *s = 1.0 / sqrt(1.0 + tau * tau);
        *c = *s * tau;
    } else {
        double tau = -b / a;
        *c = 1.0 / sqrt(1.0 + tau * tau);
        *s = *c * tau;
    }
}

static void qr_iteration(double** H, int n) {
    double c, s, t1, t2;
    for (int i = 0; i < n - 1; i++) {
        givens_rotation(H[i][i], H[i + 1][i], &c, &s);
        for (int j = i; j < n; j++) {
            t1 = c * H[i][j] - s * H[i + 1][j];
            t2 = s * H[i][j] + c * H[i + 1][j];
            H[i][j] = t1;
            H[i + 1][j] = t2;
        }
        for (int j = 0; j <= i + 1 && j < n; j++) {
            t1 = c * H[j][i] - s * H[j][i + 1];
            t2 = s * H[j][i] + c * H[j][i + 1];
            H[j][i] = t1;
            H[j][i + 1] = t2;
        }
    }
}

static double wilkinson_shift(double** H, int n) {
    double a = H[n - 2][n - 2], b = H[n - 2][n - 1];
    double c = H[n - 1][n - 2], d = H[n - 1][n - 1];
    double trace = a + d;
    double det = a * d - b * c;
    double disc = trace * trace - 4 * det;
    if (disc < 0) disc = 0;
    double sr = sqrt(disc);
    double mu1 = (trace + sr) / 2.0;
    double mu2 = (trace - sr) / 2.0;
    if (fabs(mu1 - d) < fabs(mu2 - d)) return mu1;
    return mu2;
}

StateSpace* ss_create(int n, int m, int p) {
    StateSpace* ss = (StateSpace*)malloc(sizeof(StateSpace));
    ss->n_states = n;
    ss->n_inputs = m;
    ss->n_outputs = p;
    ss->A = mat_create(n, n);
    ss->B = mat_create(n, m);
    ss->C = mat_create(p, n);
    ss->D = mat_create(p, m);
    return ss;
}

void ss_free(StateSpace* ss) {
    mat_free(ss->A, ss->n_states);
    mat_free(ss->B, ss->n_states);
    mat_free(ss->C, ss->n_outputs);
    mat_free(ss->D, ss->n_outputs);
    free(ss);
}

void ss_set_A(StateSpace* ss, int i, int j, double val) { ss->A[i][j] = val; }
void ss_set_B(StateSpace* ss, int i, int j, double val) { ss->B[i][j] = val; }
void ss_set_C(StateSpace* ss, int i, int j, double val) { ss->C[i][j] = val; }
void ss_set_D(StateSpace* ss, int i, int j, double val) { ss->D[i][j] = val; }

double* ss_step(StateSpace* ss, double* x, double* u, double dt) {
    int n = ss->n_states, m = ss->n_inputs, p = ss->n_outputs;
    double* x_dot = (double*)calloc(n, sizeof(double));
    double* y = (double*)calloc(p, sizeof(double));
    mat_vec_mul(x_dot, ss->A, x, n, n);
    double* Bu = (double*)calloc(n, sizeof(double));
    mat_vec_mul(Bu, ss->B, u, n, m);
    for (int i = 0; i < n; i++) x_dot[i] += Bu[i];
    for (int i = 0; i < n; i++) x[i] += dt * x_dot[i];
    mat_vec_mul(y, ss->C, x, p, n);
    double* Du = (double*)calloc(p, sizeof(double));
    mat_vec_mul(Du, ss->D, u, p, m);
    for (int i = 0; i < p; i++) y[i] += Du[i];
    free(x_dot); free(Bu); free(Du);
    return y;
}

double* ss_simulate(StateSpace* ss, double* x0, double** u, int steps, double dt, double** x_out) {
    int n = ss->n_states;
    double* x = (double*)malloc(n * sizeof(double));
    memcpy(x, x0, n * sizeof(double));
    if (x_out) {
        *x_out = (double*)malloc(n * (steps + 1) * sizeof(double));
        memcpy(*x_out, x0, n * sizeof(double));
    }
    for (int k = 0; k < steps; k++) {
        double* y = ss_step(ss, x, u[k], dt);
        free(y);
        if (x_out)
            memcpy(*x_out + (k + 1) * n, x, n * sizeof(double));
    }
    return x;
}

double* ss_eigenvalues(StateSpace* ss) {
    int n = ss->n_states;
    double** H = mat_create(n, n);
    mat_copy(H, ss->A, n, n);
    hessenberg_reduce(H, n);

    int max_iter = 200;
    for (int iter = 0; iter < max_iter; iter++) {
        for (int i = n - 1; i > 0; i--) {
            if (fabs(H[i][i - 1]) < 1e-12) {
                H[i][i - 1] = 0;
            }
        }
        int m = n;
        while (m > 1 && fabs(H[m - 1][m - 2]) < 1e-12) m--;

        if (m <= 1) break;

        double shift = wilkinson_shift(H, m);
        for (int i = 0; i < m; i++) H[i][i] -= shift;
        qr_iteration(H, m);
        for (int i = 0; i < m; i++) H[i][i] += shift;
    }

    double* evals = (double*)malloc(n * 2 * sizeof(double));
    int idx = 0;
    int i = 0;
    while (i < n) {
        if (i < n - 1 && fabs(H[i + 1][i]) > 1e-10) {
            double a = H[i][i], b = H[i][i + 1];
            double c = H[i + 1][i], d = H[i + 1][i + 1];
            double trace = a + d;
            double det = a * d - b * c;
            double disc = trace * trace - 4 * det;
            if (disc >= 0) {
                evals[idx * 2] = (trace + sqrt(disc)) / 2.0;
                evals[idx * 2 + 1] = 0;
            } else {
                evals[idx * 2] = trace / 2.0;
                evals[idx * 2 + 1] = sqrt(-disc) / 2.0;
            }
            idx++;
            i += 2;
        } else {
            evals[idx * 2] = H[i][i];
            evals[idx * 2 + 1] = 0;
            idx++;
            i++;
        }
    }

    mat_free(H, n);
    return evals;
}

bool ss_is_stable(StateSpace* ss) {
    double* evals = ss_eigenvalues(ss);
    bool stable = is_stable_poles(evals, ss->n_states);
    free(evals);
    return stable;
}

void ss_print(StateSpace* ss) {
    printf("StateSpace: %d states, %d inputs, %d outputs\n",
           ss->n_states, ss->n_inputs, ss->n_outputs);
    printf("A (%dx%d):\n", ss->n_states, ss->n_states);
    for (int i = 0; i < ss->n_states; i++) {
        for (int j = 0; j < ss->n_states; j++) printf(" %8.4f", ss->A[i][j]);
        printf("\n");
    }
    printf("B (%dx%d):\n", ss->n_states, ss->n_inputs);
    for (int i = 0; i < ss->n_states; i++) {
        for (int j = 0; j < ss->n_inputs; j++) printf(" %8.4f", ss->B[i][j]);
        printf("\n");
    }
    printf("C (%dx%d):\n", ss->n_outputs, ss->n_states);
    for (int i = 0; i < ss->n_outputs; i++) {
        for (int j = 0; j < ss->n_states; j++) printf(" %8.4f", ss->C[i][j]);
        printf("\n");
    }
    printf("D (%dx%d):\n", ss->n_outputs, ss->n_inputs);
    for (int i = 0; i < ss->n_outputs; i++) {
        for (int j = 0; j < ss->n_inputs; j++) printf(" %8.4f", ss->D[i][j]);
        printf("\n");
    }
}

StateSpace* ss_create_mass_spring_damper(double m, double c, double k) {
    StateSpace* ss = ss_create(2, 1, 2);
    ss_set_A(ss, 0, 1, 1.0);
    ss_set_A(ss, 1, 0, -k / m);
    ss_set_A(ss, 1, 1, -c / m);
    ss_set_B(ss, 0, 0, 0.0);
    ss_set_B(ss, 1, 0, 1.0 / m);
    ss_set_C(ss, 0, 0, 1.0);
    ss_set_C(ss, 1, 1, 1.0);
    return ss;
}

StateSpace* ss_create_pid_plant(double K, double tau) {
    StateSpace* ss = ss_create(1, 1, 1);
    ss_set_A(ss, 0, 0, -1.0 / tau);
    ss_set_B(ss, 0, 0, K / tau);
    ss_set_C(ss, 0, 0, 1.0);
    return ss;
}
