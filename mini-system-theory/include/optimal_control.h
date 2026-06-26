#ifndef OPTIMAL_CONTROL_H
#define OPTIMAL_CONTROL_H

/* L1: LQR (Linear Quadratic Regulator) problem definition
 * Minimize J = ∫ (x'Qx + u'Ru) dt subject to xdot = Ax + Bu
 * Reference: Kalman, R.E. (1960) "Contributions to the Theory of Optimal Control"
 */
typedef struct {
    double** Q;     /* State cost matrix (n x n), symmetric positive semi-definite */
    double** R;     /* Input cost matrix (m x m), symmetric positive definite */
    double** A;     /* System dynamics matrix (n x n) */
    double** B;     /* Input matrix (n x m) */
    int n_states;
    int n_inputs;
} LQRProblem;

/* L1: LQR solution containing optimal gain matrix */
typedef struct {
    double** K;     /* Optimal feedback gain (m x n), u = -Kx */
    double** P;     /* Solution to algebraic Riccati equation (n x n) */
    int n_states;
    int n_inputs;
} LQRSolution;

/* L5: Algebraic Riccati Equation solver using Kleinman's iterative method
 * A'P + PA - PBR^{-1}B'P + Q = 0
 * Complexity: O(n^3) per iteration
 */
LQRSolution* lqr_solve(LQRProblem* prob, int max_iter, double tol);
void lqr_free_solution(LQRSolution* sol);

/* L8: Discrete-time LQR — finite horizon, dynamic programming
 * Reference: Kwakernaak & Sivan (1972) Linear Optimal Control Systems
 */
double** dlqr_finite_horizon(double** A, double** B, double** Q, double** R,
                              int n, int m, int horizon, double** K_seq);

/* L1: Kalman Filter state estimator — optimal for linear Gaussian systems */
typedef struct {
    double** A;      /* State transition matrix (n x n) */
    double** B;      /* Control input matrix (n x m) */
    double** C;      /* Measurement matrix (p x n) */
    double** Q;      /* Process noise covariance (n x n) */
    double** R;      /* Measurement noise covariance (p x p) */
    double* x;       /* State estimate (n) */
    double** P;      /* Error covariance matrix (n x n) */
    int n_states;
    int n_inputs;
    int n_outputs;
} KalmanFilter;

/* L5: Kalman filter predict + update cycle
 * Predict: x = A*x + B*u, P = A*P*A' + Q
 * Update: K = P*C'*(C*P*C' + R)^{-1}
 *         x = x + K*(y - C*x), P = (I - K*C)*P
 * Reference: Kalman, R.E. (1960) "A New Approach to Linear Filtering..."
 */
KalmanFilter* kalman_create(int n, int m, int p);
void kalman_free(KalmanFilter* kf);
void kalman_set_matrices(KalmanFilter* kf, double** A, double** B, double** C,
                         double** Q, double** R);
void kalman_predict(KalmanFilter* kf, double* u);
void kalman_update(KalmanFilter* kf, double* y);
void kalman_get_state(KalmanFilter* kf, double* x_out);

/* L4: Separation principle — LQR + Kalman filter = LQG
 * Optimal control and optimal estimation can be designed independently
 * Reference: Wonham, W.M. (1968) "On the Separation Theorem of Stochastic Control"
 */
typedef struct {
    LQRSolution* lqr;
    KalmanFilter* kf;
    int n_states;
    double* x_hat;
} LQGController;

LQGController* lqg_create(LQRProblem* lqr_prob, KalmanFilter* kf);
double* lqg_step(LQGController* lqg, double* y, double* ref, double* u_out);
void lqg_free(LQGController* lqg);

/* L7: Model Predictive Control — receding horizon optimization */
typedef struct {
    double** A;
    double** B;
    double** Q;
    double** R;
    double* x_ref;
    int n_states;
    int n_inputs;
    int horizon;
    double dt;
} MPCProblem;

typedef struct {
    double** u_seq;
    double cost;
    int converged;
} MPCResult;

MPCResult* mpc_solve_unconstrained(MPCProblem* prob, double* x0);
void mpc_free_result(MPCResult* res);

#endif
