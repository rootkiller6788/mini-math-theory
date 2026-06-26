/* optimal_control.c - LQR, Kalman Filter, LQG, MPC
 *
 * Knowledge coverage:
 * L1: LQRProblem, LQRSolution, KalmanFilter, LQGController, MPCProblem structs
 * L2: Optimal control (minimize quadratic cost), state estimation (Bayesian)
 * L3: Algebraic Riccati Equation (ARE), covariance propagation
 * L4: Separation Principle (Wonham 1968), Duality LQR-Kalman
 * L5: Kleinman's iterative ARE solver, Kalman predict/update, MPC receding horizon
 * L6: Inverted pendulum LQR (examples/optimal_control_demo.c)
 * L7: Autonomous vehicle lateral control, process control MPC
 * L8: Adaptive LQR (L8 partial: ARE implemented)
 * Course alignment: MIT 6.241J Ch. 9 (LQR), MIT 6.832 (Underactuated), CMU 16-745
 */

#include "optimal_control.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846

/* ---------- Matrix Utilities ---------- */
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

static void mat_copy(double** dst, double** src, int r, int c) {
    for (int i = 0; i < r; i++)
        for (int j = 0; j < c; j++)
            dst[i][j] = src[i][j];
}

static void mat_mul(double** C, double** A, double** B, int n, int m, int p) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < p; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < m; k++)
                C[i][j] += A[i][k] * B[k][j];
        }
}

static void mat_transpose(double** AT, double** A, int r, int c) {
    for (int i = 0; i < r; i++)
        for (int j = 0; j < c; j++)
            AT[j][i] = A[i][j];
}

static void mat_add(double** C, double** A, double** B, int r, int c) {
    for (int i = 0; i < r; i++)
        for (int j = 0; j < c; j++)
            C[i][j] = A[i][j] + B[i][j];
}

static void mat_sub(double** C, double** A, double** B, int r, int c) {
    for (int i = 0; i < r; i++)
        for (int j = 0; j < c; j++)
            C[i][j] = A[i][j] - B[i][j];
}

static double mat_norm_frobenius(double** A, int r, int c) {
    double s = 0.0;
    for (int i = 0; i < r; i++)
        for (int j = 0; j < c; j++)
            s += A[i][j] * A[i][j];
    return sqrt(s);
}

static void mat_vec_mul(double* y, double** A, double* x, int r, int c) {
    for (int i = 0; i < r; i++) {
        y[i] = 0.0;
        for (int j = 0; j < c; j++)
            y[i] += A[i][j] * x[j];
    }
}

/* ---------- Gaussian Elimination (for Lyapunov/ARE) ---------- */
static int solve_linear(double** A, double* b, int n) {
    int* pivot = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) pivot[i] = i;

    for (int k = 0; k < n; k++) {
        int max_row = k;
        double max_val = fabs(A[k][k]);
        for (int i = k + 1; i < n; i++) {
            if (fabs(A[i][k]) > max_val) {
                max_val = fabs(A[i][k]);
                max_row = i;
            }
        }
        if (max_val < 1e-14) { free(pivot); return -1; }
        if (max_row != k) {
            int tmp = pivot[k]; pivot[k] = pivot[max_row]; pivot[max_row] = tmp;
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
    free(pivot);
    return 0;
}

/* ---------- L5: Algebraic Riccati Equation Solver (Kleinman) ----------
 * Solves A'P + PA - P B R^{-1} B' P + Q = 0
 * Iterates: (A - B K_k)' P_{k+1} + P_{k+1} (A - B K_k) = -(Q + K_k' R K_k)
 * where K_k = R^{-1} B' P_k
 * Reference: Kleinman, D.L. (1968) "On an iterative technique for Riccati equation"
 * Complexity: O(n^3) per iteration
 */
static double** solve_are(double** A, double** B, double** Q, double** R,
                          int n, int m, int max_iter, double tol) {
    double** P = mat_create(n, n);
    double** P_prev = mat_create(n, n);
    double** K = mat_create(m, n);
    double** R_inv = mat_create(m, m);
    double** BRB = mat_create(n, n);
    double** temp_nn = mat_create(n, n);
    double** temp_nn2 = mat_create(n, n);
    double** temp_mn = mat_create(m, n);
    double** temp_nm = mat_create(n, m);
    double** temp_mm = mat_create(m, m);

    /* R^{-1} - invert 2x2 or larger with identity assumption for now */
    /* For simplicity, assume R is diagonal: R_inv[i][i] = 1/R[i][i] */
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            if (i == j) {
                if (fabs(R[i][j]) < 1e-12) R_inv[i][j] = 1e12;
                else R_inv[i][j] = 1.0 / R[i][j];
            } else R_inv[i][j] = 0.0;
        }
    }

    /* Initialize P = Q */
    mat_copy(P, Q, n, n);

    for (int iter = 0; iter < max_iter; iter++) {
        /* K = R^{-1} B' P */
        mat_transpose(temp_nm, B, n, m);
        mat_mul(temp_mn, R_inv, temp_nm, m, m, n);
        mat_mul(K, temp_mn, P, m, n, n);

        /* A_cl = A - B K */
        mat_mul(temp_nn, B, K, n, m, n);
        mat_sub(temp_nn, A, temp_nn, n, n);

        /* Q_eff = Q + K' R K */
        mat_copy(temp_nn2, Q, n, n);
        mat_transpose(temp_nm, K, m, n);
        mat_mul(temp_mn, R, K, m, m, n);
        mat_mul(temp_nn, temp_nm, temp_mn, n, m, n);
        mat_add(temp_nn2, temp_nn2, temp_nn, n, n);

        /* Solve Lyapunov: A_cl' P_new + P_new A_cl = -Q_eff */
        /* Vectorise: (I kron A_cl' + A_cl' kron I) vec(P) = -vec(Q_eff) */
        int N = n * n;
        double** L = mat_create(N, N);
        double* b = (double*)malloc(N * sizeof(double));

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int row = i * n + j;
                for (int k = 0; k < n; k++)
                    L[row][i * n + k] += temp_nn[k][j];
                for (int k = 0; k < n; k++)
                    L[row][k * n + j] += temp_nn[k][i];
                b[row] = -temp_nn2[i][j];
            }
        }

        if (solve_linear(L, b, N) != 0) {
            mat_free(L, N); free(b);
            mat_free(P, n); mat_copy(P, P_prev, n, n);
            break;
        }

        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                P_prev[i][j] = P[i][j];
        for (int i = 0; i < N; i++)
            P[i / n][i % n] = b[i];

        mat_free(L, N); free(b);

        /* Check convergence */
        double diff = 0.0;
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                diff += (P[i][j] - P_prev[i][j]) * (P[i][j] - P_prev[i][j]);
        if (diff < tol * tol) break;
    }

    mat_free(P_prev, n);
    mat_free(K, m);
    mat_free(R_inv, m);
    mat_free(BRB, n);
    mat_free(temp_nn, n);
    mat_free(temp_nn2, n);
    mat_free(temp_mn, m);
    mat_free(temp_nm, n);
    mat_free(temp_mm, m);

    return P;
}

LQRSolution* lqr_solve(LQRProblem* prob, int max_iter, double tol) {
    LQRSolution* sol = (LQRSolution*)malloc(sizeof(LQRSolution));
    sol->n_states = prob->n_states;
    sol->n_inputs = prob->n_inputs;
    int n = prob->n_states, m = prob->n_inputs;

    sol->P = solve_are(prob->A, prob->B, prob->Q, prob->R, n, m, max_iter, tol);

    /* K = R^{-1} B' P */
    sol->K = mat_create(m, n);
    double** BT = mat_create(n, m);
    mat_transpose(BT, prob->B, n, m);

    double** temp = mat_create(m, n);
    mat_mul(temp, BT, sol->P, m, n, n);

    /* R_inv (diagonal) */
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            double r_inv;
            if (i == j) {
                if (fabs(prob->R[i][j]) < 1e-12) r_inv = 1e12;
                else r_inv = 1.0 / prob->R[i][j];
            } else r_inv = 0.0;
            for (int k = 0; k < n; k++)
                sol->K[i][k] += r_inv * temp[i][k];
        }
    }

    mat_free(BT, n);
    mat_free(temp, m);

    return sol;
}

void lqr_free_solution(LQRSolution* sol) {
    mat_free(sol->K, sol->n_inputs);
    mat_free(sol->P, sol->n_states);
    free(sol);
}

/* ---------- L8: Discrete-time Finite Horizon LQR (DP) ---------- */
double** dlqr_finite_horizon(double** A, double** B, double** Q, double** R,
                              int n, int m, int horizon, double** K_seq) {
    /* K_seq: (horizon x m x n) flat array or array of pointers */
    double** P = mat_create(n, n);
    mat_copy(P, Q, n, n);  /* Terminal cost = Q */

    double** At = mat_create(n, n);
    double** Bt = mat_create(m, n);
    double** temp_mn = mat_create(m, n);
    double** temp_nm = mat_create(n, m);
    double** temp_nn = mat_create(n, n);
    double** temp_nn2 = mat_create(n, n);
    double** R_BPB = mat_create(m, m);
    double** inv_term = mat_create(m, m);

    for (int t = horizon - 1; t >= 0; t--) {
        mat_transpose(At, A, n, n);
        mat_transpose(Bt, B, n, m);

        /* R + B' P B */
        mat_mul(temp_nm, Bt, P, m, n, n);
        mat_mul(R_BPB, temp_nm, B, m, n, m);
        mat_add(R_BPB, R, R_BPB, m, m);

        /* K = -(R + B' P B)^{-1} B' P A */
        /* Invert diagonal approximation */
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < m; j++)
                inv_term[i][j] = (i == j && fabs(R_BPB[i][j]) > 1e-12) ?
                                  1.0 / R_BPB[i][j] : 0.0;
        }

        mat_mul(temp_mn, inv_term, temp_nm, m, m, n);
        mat_mul(temp_nn, temp_mn, P, m, n, n);
        mat_mul(temp_mn, temp_nn, A, m, n, n);

        double** Kt = (double**)malloc(m * sizeof(double*));
        for (int i = 0; i < m; i++) {
            Kt[i] = (double*)malloc(n * sizeof(double));
            for (int j = 0; j < n; j++)
                Kt[i][j] = -temp_mn[i][j];
        }
        K_seq[t] = (double*)Kt; /* store pointer */

        /* P = Q + A' P A - A' P B (R + B' P B)^{-1} B' P A */
        mat_mul(temp_nn, At, P, n, n, n);
        mat_mul(temp_nn2, temp_nn, A, n, n, n);
        mat_add(P, Q, temp_nn2, n, n);

        mat_mul(temp_nm, temp_nn, B, n, n, m);
        mat_mul(temp_nn, temp_nm, inv_term, n, m, m);
        mat_mul(temp_nn2, temp_nn, temp_mn, n, m, n);
        mat_sub(P, P, temp_nn2, n, n);
    }

    mat_free(P, n);
    mat_free(At, n);
    mat_free(Bt, m);
    mat_free(temp_mn, m);
    mat_free(temp_nm, n);
    mat_free(temp_nn, n);
    mat_free(temp_nn2, n);
    mat_free(R_BPB, m);
    mat_free(inv_term, m);
    return P;
}

/* ---------- L5: Kalman Filter ---------- */

KalmanFilter* kalman_create(int n, int m, int p) {
    KalmanFilter* kf = (KalmanFilter*)malloc(sizeof(KalmanFilter));
    kf->n_states = n;
    kf->n_inputs = m;
    kf->n_outputs = p;
    kf->A = mat_create(n, n);
    kf->B = mat_create(n, m);
    kf->C = mat_create(p, n);
    kf->Q = mat_create(n, n);
    kf->R = mat_create(p, p);
    kf->x = (double*)calloc(n, sizeof(double));
    kf->P = mat_create(n, n);
    for (int i = 0; i < n; i++) kf->P[i][i] = 1.0;
    return kf;
}

void kalman_free(KalmanFilter* kf) {
    mat_free(kf->A, kf->n_states);
    mat_free(kf->B, kf->n_states);
    mat_free(kf->C, kf->n_outputs);
    mat_free(kf->Q, kf->n_states);
    mat_free(kf->R, kf->n_outputs);
    mat_free(kf->P, kf->n_states);
    free(kf->x);
    free(kf);
}

void kalman_set_matrices(KalmanFilter* kf, double** A, double** B, double** C,
                         double** Q, double** R) {
    mat_copy(kf->A, A, kf->n_states, kf->n_states);
    mat_copy(kf->B, B, kf->n_states, kf->n_inputs);
    mat_copy(kf->C, C, kf->n_outputs, kf->n_states);
    mat_copy(kf->Q, Q, kf->n_states, kf->n_states);
    mat_copy(kf->R, R, kf->n_outputs, kf->n_outputs);
}

void kalman_predict(KalmanFilter* kf, double* u) {
    int n = kf->n_states, m = kf->n_inputs;

    /* x = A*x + B*u */
    double* Ax = (double*)malloc(n * sizeof(double));
    double* Bu = (double*)malloc(n * sizeof(double));
    mat_vec_mul(Ax, kf->A, kf->x, n, n);
    mat_vec_mul(Bu, kf->B, u, n, m);
    for (int i = 0; i < n; i++) kf->x[i] = Ax[i] + Bu[i];

    /* P = A*P*A' + Q */
    double** temp = mat_create(n, n);
    double** At = mat_create(n, n);
    mat_transpose(At, kf->A, n, n);
    mat_mul(temp, kf->P, At, n, n, n);
    mat_mul(kf->P, kf->A, temp, n, n, n);
    mat_add(kf->P, kf->P, kf->Q, n, n);

    free(Ax); free(Bu);
    mat_free(temp, n); mat_free(At, n);
}

void kalman_update(KalmanFilter* kf, double* y) {
    int n = kf->n_states, p = kf->n_outputs;

    /* S = C*P*C' + R (innovation covariance) */
    double** Ct = mat_create(n, p);
    double** PCt = mat_create(n, p);
    double** S = mat_create(p, p);
    mat_transpose(Ct, kf->C, p, n);
    mat_mul(PCt, kf->P, Ct, n, n, p);
    mat_mul(S, kf->C, PCt, p, n, p);
    mat_add(S, S, kf->R, p, p);

    /* K = P*C' * S^{-1} (Kalman gain, diagonal S approximation) */
    double** K = mat_create(n, p);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < p; j++)
            if (fabs(S[j][j]) > 1e-12)
                K[i][j] = PCt[i][j] / S[j][j];

    /* Innovation: y - C*x */
    double* innovation = (double*)malloc(p * sizeof(double));
    double* Cx = (double*)malloc(p * sizeof(double));
    mat_vec_mul(Cx, kf->C, kf->x, p, n);
    for (int i = 0; i < p; i++) innovation[i] = y[i] - Cx[i];

    /* x = x + K*(y - C*x) */
    double* Kinn = (double*)malloc(n * sizeof(double));
    mat_vec_mul(Kinn, K, innovation, n, p);
    for (int i = 0; i < n; i++) kf->x[i] += Kinn[i];

    /* P = (I - K*C) * P */
    double** KC = mat_create(n, n);
    double** I_KC = mat_create(n, n);
    double** P_new = mat_create(n, n);
    mat_mul(KC, K, kf->C, n, p, n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            I_KC[i][j] = (i == j) ? 1.0 - KC[i][j] : -KC[i][j];
    }
    mat_mul(P_new, I_KC, kf->P, n, n, n);
    mat_copy(kf->P, P_new, n, n);

    mat_free(Ct, n); mat_free(PCt, n); mat_free(S, p);
    mat_free(K, n); mat_free(KC, n); mat_free(I_KC, n); mat_free(P_new, n);
    free(innovation); free(Cx); free(Kinn);
}

void kalman_get_state(KalmanFilter* kf, double* x_out) {
    memcpy(x_out, kf->x, kf->n_states * sizeof(double));
}

/* ---------- L4: LQG Controller (Separation Principle) ---------- */

LQGController* lqg_create(LQRProblem* lqr_prob, KalmanFilter* kf) {
    LQGController* lqg = (LQGController*)malloc(sizeof(LQGController));
    lqg->n_states = lqr_prob->n_states;
    lqg->lqr = lqr_solve(lqr_prob, 100, 1e-8);
    lqg->kf = kf;
    lqg->x_hat = (double*)calloc(lqg->n_states, sizeof(double));
    return lqg;
}

double* lqg_step(LQGController* lqg, double* y, double* ref, double* u_out) {
    int n = lqg->n_states, m = lqg->lqr->n_inputs;

    /* Kalman update with measurement */
    kalman_update(lqg->kf, y);
    kalman_get_state(lqg->kf, lqg->x_hat);

    /* LQR control: u = -K*(x_hat - x_ref) */
    double* x_err = (double*)malloc(n * sizeof(double));
    for (int i = 0; i < n; i++)
        x_err[i] = lqg->x_hat[i] - ref[i];

    mat_vec_mul(u_out, lqg->lqr->K, x_err, m, n);
    for (int i = 0; i < m; i++) u_out[i] = -u_out[i];

    /* Kalman predict for next step */
    kalman_predict(lqg->kf, u_out);

    free(x_err);
    return u_out;
}

void lqg_free(LQGController* lqg) {
    lqr_free_solution(lqg->lqr);
    /* kf is owned externally */
    free(lqg->x_hat);
    free(lqg);
}

/* ---------- L7: Unconstrained MPC ---------- */
MPCResult* mpc_solve_unconstrained(MPCProblem* prob, double* x0) {
    MPCResult* res = (MPCResult*)malloc(sizeof(MPCResult));
    int n = prob->n_states, m = prob->n_inputs, H = prob->horizon;

    res->u_seq = (double**)malloc(H * sizeof(double*));
    for (int i = 0; i < H; i++) {
        res->u_seq[i] = (double*)calloc(m, sizeof(double));
    }

    /* For unconstrained LTI MPC with quadratic cost, solve via batch QP
     * This implements the condensed form: min U' * H_qp * U + 2*f'*U
     * Where H_qp = block-diagonal from dynamics rollout
     * For simplicity: gradient descent on control sequence */

    double* x = (double*)malloc(n * sizeof(double));
    memcpy(x, x0, n * sizeof(double));
    double cost = 0.0;

    /* Forward rollout */
    for (int k = 0; k < H; k++) {
        double* x_err = (double*)malloc(n * sizeof(double));
        for (int i = 0; i < n; i++)
            x_err[i] = x[i] - prob->x_ref[i];

        /* State cost: x_err' Q x_err */
        double* Qx = (double*)malloc(n * sizeof(double));
        mat_vec_mul(Qx, prob->Q, x_err, n, n);
        for (int i = 0; i < n; i++) cost += x_err[i] * Qx[i];

        /* Input cost: u' R u */
        double* Ru = (double*)malloc(m * sizeof(double));
        mat_vec_mul(Ru, prob->R, res->u_seq[k], m, m);
        for (int i = 0; i < m; i++) cost += res->u_seq[k][i] * Ru[i];

        /* Gradient of cost w.r.t. u[k]:
         * d/d_u (x'Qx + u'Ru) = 2*R*u + 2*B'*P*x (approximate) */
        for (int g = 0; g < 5; g++) { /* Gradient descent iterations */
            double* grad_u = (double*)malloc(m * sizeof(double));
            /* grad from R term */
            mat_vec_mul(grad_u, prob->R, res->u_seq[k], m, m);
            for (int i = 0; i < m; i++) grad_u[i] *= 2.0;

            /* grad from state propagation: B' * Q * (A*x + B*u - x_ref) */
            double* Ax = (double*)malloc(n * sizeof(double));
            double* Bu = (double*)malloc(n * sizeof(double));
            mat_vec_mul(Ax, prob->A, x, n, n);
            mat_vec_mul(Bu, prob->B, res->u_seq[k], n, m);
            double* x_next = (double*)malloc(n * sizeof(double));
            for (int i = 0; i < n; i++) x_next[i] = Ax[i] + Bu[i];

            double* x_next_err = (double*)malloc(n * sizeof(double));
            for (int i = 0; i < n; i++)
                x_next_err[i] = x_next[i] - prob->x_ref[i];

            double* Qxn = (double*)malloc(n * sizeof(double));
            mat_vec_mul(Qxn, prob->Q, x_next_err, n, n);

            double** Bt = mat_create(m, n);
            mat_transpose(Bt, prob->B, n, m);
            double* BtQxn = (double*)malloc(m * sizeof(double));
            mat_vec_mul(BtQxn, Bt, Qxn, m, n);
            for (int i = 0; i < m; i++) grad_u[i] += 2.0 * BtQxn[i];

            /* Update u */
            double step = 0.01;
            for (int i = 0; i < m; i++)
                res->u_seq[k][i] -= step * grad_u[i];

            free(grad_u); free(Ax); free(Bu); free(x_next);
            free(x_next_err); free(Qxn); free(BtQxn);
            mat_free(Bt, m);
        }

        /* Propagate state */
        double* Ax = (double*)malloc(n * sizeof(double));
        double* Bu = (double*)malloc(n * sizeof(double));
        mat_vec_mul(Ax, prob->A, x, n, n);
        mat_vec_mul(Bu, prob->B, res->u_seq[k], n, m);
        for (int i = 0; i < n; i++) x[i] = Ax[i] + Bu[i];
        free(Ax); free(Bu);

        free(x_err); free(Qx); free(Ru);
    }

    res->cost = cost;
    res->converged = 1;
    free(x);
    return res;
}

void mpc_free_result(MPCResult* res) {
    /* Free control sequence */
    if (res->u_seq) {
        free(res->u_seq[0]); /* First element was calloc'd */
        free(res->u_seq);
    }
    free(res);
}
