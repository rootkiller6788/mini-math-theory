#include "stability.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PI 3.14159265358979323846

static double** mat_create(int r, int c) {
    double** m = (double**)malloc(r * sizeof(double*));
    for (int i = 0; i < r; i++)
        m[i] = (double*)calloc(c, sizeof(double));
    return m;
}

static void mat_free(double** m, int r) {
    for (int i = 0; i < r; i++) free(m[i]);
    free(m);
}

bool is_stable_poles(double* eigenvalues, int n) {
    for (int i = 0; i < n; i++) {
        if (eigenvalues[2 * i] >= 0) return false;
    }
    return true;
}

bool is_bibo_stable(double* impulse_response, int n, double tol) {
    double sum = 0;
    for (int i = 0; i < n; i++)
        sum += fabs(impulse_response[i]);
    return sum < 1.0 / tol;
}

static void gauss_eliminate(double** A, double* b, int n) {
    for (int k = 0; k < n; k++) {
        int max_row = k;
        for (int i = k + 1; i < n; i++)
            if (fabs(A[i][k]) > fabs(A[max_row][k])) max_row = i;
        if (fabs(A[max_row][k]) < 1e-14) continue;
        if (max_row != k) {
            double* tmp_r = A[k]; A[k] = A[max_row]; A[max_row] = tmp_r;
            double tmp_b = b[k]; b[k] = b[max_row]; b[max_row] = tmp_b;
        }
        for (int i = k + 1; i < n; i++) {
            double factor = A[i][k] / A[k][k];
            for (int j = k; j < n; j++)
                A[i][j] -= factor * A[k][j];
            b[i] -= factor * b[k];
        }
    }
    for (int i = n - 1; i >= 0; i--) {
        double sum = b[i];
        for (int j = i + 1; j < n; j++)
            sum -= A[i][j] * b[j];
        b[i] = sum / A[i][i];
    }
}

double* lyapunov_solve(double** A, int n, double** Q) {
    int N = n * n;
    double** L = (double**)malloc(N * sizeof(double*));
    for (int i = 0; i < N; i++)
        L[i] = (double*)calloc(N, sizeof(double));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int row = i * n + j;
            for (int k = 0; k < n; k++)
                L[row][i * n + k] += A[i][k];
            for (int k = 0; k < n; k++)
                L[row][k * n + j] += A[j][k];
        }
    }

    double* b = (double*)malloc(N * sizeof(double));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            b[i * n + j] = -Q[i][j];

    gauss_eliminate(L, b, N);
    double* X = (double*)malloc(N * sizeof(double));
    memcpy(X, b, N * sizeof(double));

    free(b);
    for (int i = 0; i < N; i++) free(L[i]);
    free(L);
    return X;
}

static void poly_eval_complex(double* num, int n_num, double* den, int n_den,
                               double omega, double* mag, double* phase) {
    double num_r = 0, num_i = 0;
    double den_r = 0, den_i = 0;
    for (int k = 0; k < n_num; k++) {
        int pwr = n_num - 1 - k;
        double re = (pwr % 2 == 0) ? cos(pwr * atan2(0, 1)) * pow(omega, pwr) : 0;
        double im = (pwr % 2 == 1) ? ((pwr - 1) % 4 == 0 ? pow(omega, pwr) : -pow(omega, pwr)) : 0;
        if (pwr % 4 == 0) { re = pow(omega, pwr); im = 0; }
        else if (pwr % 4 == 1) { re = 0; im = pow(omega, pwr); }
        else if (pwr % 4 == 2) { re = -pow(omega, pwr); im = 0; }
        else { re = 0; im = -pow(omega, pwr); }
        num_r += num[k] * re;
        num_i += num[k] * im;
    }
    for (int k = 0; k < n_den; k++) {
        int pwr = n_den - 1 - k;
        double re = 0, im = 0;
        if (pwr % 4 == 0) { re = pow(omega, pwr); im = 0; }
        else if (pwr % 4 == 1) { re = 0; im = pow(omega, pwr); }
        else if (pwr % 4 == 2) { re = -pow(omega, pwr); im = 0; }
        else { re = 0; im = -pow(omega, pwr); }
        den_r += den[k] * re;
        den_i += den[k] * im;
    }
    double den_mag2 = den_r * den_r + den_i * den_i;
    double G_r = (num_r * den_r + num_i * den_i) / den_mag2;
    double G_i = (num_i * den_r - num_r * den_i) / den_mag2;
    *mag = sqrt(G_r * G_r + G_i * G_i);
    *phase = atan2(G_i, G_r) * 180.0 / PI;
}

void bode_margins(double* num, int n_num, double* den, int n_den,
                  double* gain_margin, double* phase_margin) {
    double w_pc = -1, w_gc = -1;
    double phase_at_gc = 0;

    for (int i = 0; i < 10000; i++) {
        double omega = 0.001 * pow(1.015, i);
        if (omega > 1000) break;
        double mag, phase;
        poly_eval_complex(num, n_num, den, n_den, omega, &mag, &phase);
        while (phase > 180) phase -= 360;
        while (phase < -360) phase += 360;
        if (w_pc < 0 && phase <= -180.0) w_pc = omega;
        if (w_gc < 0 && mag <= 1.0) {
            w_gc = omega;
            phase_at_gc = phase;
        }
        if (w_pc > 0 && w_gc > 0) break;
    }

    if (w_pc > 0) {
        double mag_pc;
        double ph;
        poly_eval_complex(num, n_num, den, n_den, w_pc, &mag_pc, &ph);
        *gain_margin = -20.0 * log10(mag_pc);
    } else {
        *gain_margin = 1e9;
    }

    *phase_margin = phase_at_gc + 180.0;
    if (*phase_margin > 360) *phase_margin -= 360;
}

static double** mat_copy_create(double** src, int n) {
    double** dst = mat_create(n, n);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            dst[i][j] = src[i][j];
    return dst;
}

double* root_locus_eig(double** A, double** B, double** C, int n, double K) {
    double** Ac = mat_create(n, n);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            Ac[i][j] = A[i][j];
            for (int k = 0; k < n; k++)
                Ac[i][j] -= K * B[i][0] * C[0][k] * ((j == k) ? 1.0 : 0.0);
        }

    double* evals = (double*)malloc(2 * n * sizeof(double));
    if (n == 1) {
        evals[0] = Ac[0][0];
        evals[1] = 0;
    } else if (n == 2) {
        double a = Ac[0][0], b = Ac[0][1], c = Ac[1][0], d = Ac[1][1];
        double trace = a + d, det = a * d - b * c;
        double disc = trace * trace - 4 * det;
        if (disc >= 0) {
            evals[0] = (trace + sqrt(disc)) / 2.0;
            evals[1] = 0;
            evals[2] = (trace - sqrt(disc)) / 2.0;
            evals[3] = 0;
        } else {
            evals[0] = trace / 2.0;
            evals[1] = sqrt(-disc) / 2.0;
            evals[2] = trace / 2.0;
            evals[3] = -sqrt(-disc) / 2.0;
        }
    } else {
        double trace = 0;
        for (int i = 0; i < n; i++) trace += Ac[i][i];
        for (int i = 0; i < n; i++) evals[2 * i] = trace / n;
        for (int i = 0; i < n; i++) evals[2 * i + 1] = 0;
    }

    mat_free(Ac, n);
    return evals;
}
