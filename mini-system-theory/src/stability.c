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
/* additional_stability.c - appended to stability.c
   Extra functions for stability analysis */

/* ---------- L4: Nyquist Stability Criterion Implementation ----------
 * Computes the number of encirclements of -1 by G(jw) for w in [0, inf)
 * and determines closed-loop stability.
 * Reference: Nyquist, H. (1932), Regeneration Theory
 */
int stability_nyquist_check(double* num, int num_len, double* den, int den_len) {
    /* Evaluate G(jw) along the Nyquist contour */
    double prev_re = 0.0, prev_im = 0.0;
    int encirclements = 0;
    bool crossed_pos = false, crossed_neg = false;

    for (int i = 0; i < 50000; i++) {
        double omega = 0.0001 * pow(1.0003, i);
        if (omega > 10000.0) break;

        /* Compute G(jw) = num(jw)/den(jw) */
        double num_r = 0.0, num_i = 0.0;
        double den_r = 0.0, den_i = 0.0;

        for (int k = 0; k < num_len; k++) {
            int pwr = num_len - 1 - k;
            double re, im;
            if (pwr % 4 == 0)      { re = pow(omega, pwr); im = 0.0; }
            else if (pwr % 4 == 1) { re = 0.0; im = pow(omega, pwr); }
            else if (pwr % 4 == 2) { re = -pow(omega, pwr); im = 0.0; }
            else                   { re = 0.0; im = -pow(omega, pwr); }
            num_r += num[k] * re; num_i += num[k] * im;
        }
        for (int k = 0; k < den_len; k++) {
            int pwr = den_len - 1 - k;
            double re, im;
            if (pwr % 4 == 0)      { re = pow(omega, pwr); im = 0.0; }
            else if (pwr % 4 == 1) { re = 0.0; im = pow(omega, pwr); }
            else if (pwr % 4 == 2) { re = -pow(omega, pwr); im = 0.0; }
            else                   { re = 0.0; im = -pow(omega, pwr); }
            den_r += den[k] * re; den_i += den[k] * im;
        }

        double den_mag2 = den_r * den_r + den_i * den_i;
        if (den_mag2 < 1e-15) continue;
        double Gr = (num_r * den_r + num_i * den_i) / den_mag2;
        double Gi = (num_i * den_r - num_r * den_i) / den_mag2;

        /* Detect crossing of negative real axis left of -1
         * Cross from Im > 0 to Im < 0: crossing right to left -> +1
         * Cross from Im < 0 to Im > 0: crossing left to right -> -1 */
        if (i > 0 && Gr < -1.0) {
            if (prev_im > 0 && Gi < 0) encirclements++;
            if (prev_im < 0 && Gi > 0) encirclements--;
        }

        prev_re = Gr; prev_im = Gi;
    }

    return encirclements;
}

/* L4: Kharitonov Robust Stability Test
 * For interval polynomial P(s) = sum a_i s^i with a_i in [a_i_min, a_i_max],
 * the family is Hurwitz-stable iff four specific polynomials are stable.
 * This function implements the Routh-Hurwitz criterion for one polynomial.
 */
int stability_routh_hurwitz(double* coeffs, int degree) {
    /* Build Routh table */
    int rows = degree + 1;
    int cols = (degree + 2) / 2;
    double* table = (double*)calloc(rows * cols, sizeof(double));

    /* Fill first two rows */
    for (int k = degree; k >= 0; k--) {
        int col = k / 2;
        int row = (degree - k) % 2;
        table[row * cols + col] = coeffs[degree - k];
    }

    /* Compute remaining rows */
    for (int i = 2; i < rows; i++) {
        for (int j = 0; j < cols - 1; j++) {
            double a = table[(i-2)*cols + j+1];
            double b = table[(i-1)*cols + j+1];
            double c = table[(i-2)*cols];
            double d = table[(i-1)*cols];
            if (fabs(d) < 1e-15) d = 1e-15;
            table[i*cols + j] = (d * a - c * b) / d;
        }
    }

    /* Check first column for sign changes */
    int sign_changes = 0;
    double prev_nonzero = table[0];
    for (int i = 1; i < rows; i++) {
        double cur = table[i*cols];
        if (fabs(cur) > 1e-12) {
            if (prev_nonzero * cur < 0) sign_changes++;
            prev_nonzero = cur;
        }
    }

    free(table);
    return sign_changes; /* 0 means stable, >0 means unstable with N RHP poles */
}

/* L5: Routh-Hurwitz stability test for Kharitonov polynomials */
int stability_kharitonov_test(double* coeffs_min, double* coeffs_max, int degree) {
    /* Build 4 Kharitonov polynomials */
    double K[4][8]; /* up to degree 7 */
    for (int v = 0; v < 4; v++) {
        for (int k = 0; k <= degree; k++) {
            int parity_even = (k % 2 == 0);
            int parity_mod4 = k % 4;

            if (v == 0) {
                /* K1: even=min, odd=min */
                K[v][k] = (k % 2 == 0) ? coeffs_min[k] : coeffs_min[k];
            } else if (v == 1) {
                /* K2: even k%4==0: min, k%4==2: max; odd: max for k%4==1, min for k%4==3 */
                if (parity_even) {
                    K[v][k] = (parity_mod4 == 0) ? coeffs_min[k] : coeffs_max[k];
                } else {
                    K[v][k] = (parity_mod4 == 1) ? coeffs_max[k] : coeffs_min[k];
                }
            } else if (v == 2) {
                /* K3: even k%4==0: max, k%4==2: min; odd k%4==1: min, k%4==3: max */
                if (parity_even) {
                    K[v][k] = (parity_mod4 == 0) ? coeffs_max[k] : coeffs_min[k];
                } else {
                    K[v][k] = (parity_mod4 == 1) ? coeffs_min[k] : coeffs_max[k];
                }
            } else {
                /* K4: even=max, odd=max */
                K[v][k] = (k % 2 == 0) ? coeffs_max[k] : coeffs_max[k];
            }
        }

        /* Test each polynomial */
        if (stability_routh_hurwitz(K[v], degree) != 0) {
            return 0; /* Unstable polynomial found */
        }
    }
    return 1; /* All 4 polynomials stable */
}

/* L8: Gain and phase margin computation via transfer function
 * GM = -20*log10(|G(jw_pc)|) where w_pc is phase crossover (phase = -180)
 * PM = angle(G(jw_gc)) + 180 where w_gc is gain crossover (|G| = 1) */
void stability_margins(double* num, int num_len, double* den, int den_len,
                       double* gm_db, double* pm_deg) {
    double w_pc = -1.0, w_gc = -1.0;
    double phase_at_gc = 0.0;

    for (int i = 0; i < 100000; i++) {
        double omega = 0.0001 * pow(1.00015, i);
        if (omega > 10000.0) break;

        double num_r = 0.0, num_i = 0.0;
        double den_r = 0.0, den_i = 0.0;

        for (int k = 0; k < num_len; k++) {
            int pwr = num_len - 1 - k;
            double re, im;
            if (pwr % 4 == 0)      { re = pow(omega, pwr); im = 0.0; }
            else if (pwr % 4 == 1) { re = 0.0; im = pow(omega, pwr); }
            else if (pwr % 4 == 2) { re = -pow(omega, pwr); im = 0.0; }
            else                   { re = 0.0; im = -pow(omega, pwr); }
            num_r += num[k] * re; num_i += num[k] * im;
        }
        for (int k = 0; k < den_len; k++) {
            int pwr = den_len - 1 - k;
            double re, im;
            if (pwr % 4 == 0)      { re = pow(omega, pwr); im = 0.0; }
            else if (pwr % 4 == 1) { re = 0.0; im = pow(omega, pwr); }
            else if (pwr % 4 == 2) { re = -pow(omega, pwr); im = 0.0; }
            else                   { re = 0.0; im = -pow(omega, pwr); }
            den_r += den[k] * re; den_i += den[k] * im;
        }

        double den_mag2 = den_r * den_r + den_i * den_i;
        if (den_mag2 < 1e-15) continue;
        double Gr = (num_r * den_r + num_i * den_i) / den_mag2;
        double Gi = (num_i * den_r - num_r * den_i) / den_mag2;
        double mag = sqrt(Gr * Gr + Gi * Gi);
        double phase = atan2(Gi, Gr) * 180.0 / PI;

        while (phase > 180.0) phase -= 360.0;
        while (phase < -180.0) phase += 360.0;

        if (w_pc < 0 && phase <= -180.0 && phase > -181.0) w_pc = omega;
        if (w_gc < 0 && fabs(mag - 1.0) < 0.01) {
            w_gc = omega;
            phase_at_gc = phase;
        }
    }

    if (w_pc > 0) {
        double num_r = 0.0, num_i = 0.0;
        double den_r = 0.0, den_i = 0.0;
        for (int k = 0; k < num_len; k++) {
            int pwr = num_len - 1 - k;
            double re, im;
            if (pwr % 4 == 0)      { re = pow(w_pc, pwr); im = 0.0; }
            else if (pwr % 4 == 1) { re = 0.0; im = pow(w_pc, pwr); }
            else if (pwr % 4 == 2) { re = -pow(w_pc, pwr); im = 0.0; }
            else                   { re = 0.0; im = -pow(w_pc, pwr); }
            num_r += num[k] * re; num_i += num[k] * im;
        }
        for (int k = 0; k < den_len; k++) {
            int pwr = den_len - 1 - k;
            double re, im;
            if (pwr % 4 == 0)      { re = pow(w_pc, pwr); im = 0.0; }
            else if (pwr % 4 == 1) { re = 0.0; im = pow(w_pc, pwr); }
            else if (pwr % 4 == 2) { re = -pow(w_pc, pwr); im = 0.0; }
            else                   { re = 0.0; im = -pow(w_pc, pwr); }
            den_r += den[k] * re; den_i += den[k] * im;
        }
        double den_mag2 = den_r * den_r + den_i * den_i;
        if (den_mag2 > 1e-15) {
            double Gr = (num_r * den_r + num_i * den_i) / den_mag2;
            double Gi = (num_i * den_r - num_r * den_i) / den_mag2;
            double mag_pc = sqrt(Gr * Gr + Gi * Gi);
            *gm_db = -20.0 * log10(mag_pc);
        } else *gm_db = INFINITY;
    } else {
        *gm_db = INFINITY;
    }

    *pm_deg = phase_at_gc + 180.0;
    if (*pm_deg > 360.0) *pm_deg -= 360.0;
}

/* L4: Lyapunov equation solver via Bartels-Stewart algorithm (simplified)
 * Solves A*X + X*A' = -Q for X (continuous-time)
 * For small matrices, uses direct Kronecker product method
 */
void stability_lyapunov_solve_direct(double** A, double** Q, int n, double** X) {
    int N = n * n;
    double** L = (double**)malloc(N * sizeof(double*));
    for (int i = 0; i < N; i++)
        L[i] = (double*)calloc(N, sizeof(double));

    /* Build (I kron A + A kron I) */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int row = i * n + j;
            for (int k = 0; k < n; k++)
                L[row][i * n + k] += A[k][j];
            for (int k = 0; k < n; k++)
                L[row][k * n + j] += A[k][i];
        }
    }

    /* RHS: vec(-Q) */
    double* rhs = (double*)malloc(N * sizeof(double));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            rhs[i * n + j] = -Q[i][j];

    /* Gaussian elimination */
    for (int k = 0; k < N; k++) {
        int max_row = k;
        for (int r = k + 1; r < N; r++)
            if (fabs(L[r][k]) > fabs(L[max_row][k]))
                max_row = r;
        if (fabs(L[max_row][k]) < 1e-14) continue;
        if (max_row != k) {
            double* tmp = L[k]; L[k] = L[max_row]; L[max_row] = tmp;
            double tb = rhs[k]; rhs[k] = rhs[max_row]; rhs[max_row] = tb;
        }
        for (int r = k + 1; r < N; r++) {
            double f = L[r][k] / L[k][k];
            for (int c = k; c < N; c++)
                L[r][c] -= f * L[k][c];
            rhs[r] -= f * rhs[k];
        }
    }
    for (int i = N - 1; i >= 0; i--) {
        double s = rhs[i];
        for (int j = i + 1; j < N; j++)
            s -= L[i][j] * rhs[j];
        rhs[i] = s / L[i][i];
    }

    /* Unvec */
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            X[i][j] = rhs[i * n + j];

    for (int i = 0; i < N; i++) free(L[i]);
    free(L);
    free(rhs);
}
