/* regression.c — Regression analysis: linear, logistic, multiple, polynomial, ridge.
 *
 * L5: Simple Linear Regression (OLS)
 * L5: Multiple Linear Regression (OLS with normal equations)
 * L5: Logistic Regression (binary, gradient descent)
 * L5: Polynomial Regression (via OLS with Vandermonde features)
 * L8: Ridge Regression (L2 regularization)
 *
 * Theorem sources:
 *   Gauss-Markov: OLS is BLUE under classical assumptions
 *   MIT 18.05 §12 (Linear Regression)
 *   CS229 §2 (Regression, classification)
 *   Hoerl & Kennard (1970): Ridge Regression
 */

#include "regression.h"
#include "inference.h"
#include "distribution.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* --------------------------------------------------------------------------
 * L5: Simple Linear Regression (OLS)
 *
 * Model: yᵢ = β₀ + β₁ xᵢ + εᵢ,  εᵢ ~ N(0,σ²)
 *
 * β₁ = SS_{xy} / SS_{xx}
 * β₀ = ȳ - β₁ x̄
 *
 * R² = 1 - SS_res / SS_tot
 * SE(β₁) = √(MSE / SS_{xx})
 * t = β₁ / SE(β₁) ~ t_{n-2}
 * -------------------------------------------------------------------------- */
LinearRegression* linreg_fit(double* x, double* y, int n) {
    if (n < 2) return NULL;
    LinearRegression* lr = (LinearRegression*)malloc(sizeof(LinearRegression));
    if (!lr) return NULL;

    double xbar = mean(x, n);
    double ybar = mean(y, n);
    double ssxx = 0.0, ssxy = 0.0, ssyy = 0.0;

    for (int i = 0; i < n; i++) {
        double dx = x[i] - xbar;
        double dy = y[i] - ybar;
        ssxx += dx * dx;
        ssxy += dx * dy;
        ssyy += dy * dy;
    }

    if (ssxx == 0.0) {
        lr->beta0 = ybar;
        lr->beta1 = 0.0;
        lr->r_squared = 0.0;
        lr->se_beta1 = 0.0;
        lr->t_stat = 0.0;
        lr->p_value = 1.0;
        return lr;
    }

    lr->beta1 = ssxy / ssxx;
    lr->beta0 = ybar - lr->beta1 * xbar;

    double ss_res = 0.0;
    for (int i = 0; i < n; i++) {
        double pred = lr->beta0 + lr->beta1 * x[i];
        double res = y[i] - pred;
        ss_res += res * res;
    }

    lr->r_squared = 1.0 - ss_res / ssyy;
    if (lr->r_squared < 0.0) lr->r_squared = 0.0;
    if (lr->r_squared > 1.0) lr->r_squared = 1.0;

    double mse = ss_res / (n - 2);
    lr->se_beta1 = sqrt(mse / ssxx);
    lr->t_stat = lr->beta1 / lr->se_beta1;

    double abs_t = fabs(lr->t_stat);
    lr->p_value = 2.0 * (1.0 - (0.5 * (1.0 + erf(abs_t / sqrt(2.0)))));

    return lr;
}

double linreg_predict(LinearRegression* lr, double x) {
    if (!lr) return 0.0;
    return lr->beta0 + lr->beta1 * x;
}

void linreg_print(LinearRegression* lr) {
    if (!lr) return;
    printf("Linear Regression: y = %.4f + %.4f * x\n", lr->beta0, lr->beta1);
    printf("  R-squared: %.4f\n", lr->r_squared);
    printf("  SE(beta1): %.4f\n", lr->se_beta1);
    printf("  t-stat: %.4f, p-value: %.6f\n", lr->t_stat, lr->p_value);
}

void linreg_free(LinearRegression* lr) {
    free(lr);
}

double* linreg_residuals(double* x, double* y, int n, double beta0, double beta1) {
    if (n <= 0) return NULL;
    double* res = (double*)malloc(n * sizeof(double));
    if (!res) return NULL;
    for (int i = 0; i < n; i++) {
        res[i] = y[i] - (beta0 + beta1 * x[i]);
    }
    return res;
}

/* --------------------------------------------------------------------------
 * L5: Multiple Linear Regression via Normal Equations
 *
 * Model: y = Xβ + ε
 * Solution: β̂ = (X^T X)^{-1} X^T y
 *
 * X is n×(p+1) design matrix (column 0 → intercept of 1s)
 * β is (p+1)×1 coefficient vector
 *
 * Uses Gaussian elimination for 2×2 inverse (p ≤ 2 for stability)
 * Complexity: O(np² + p³)
 * -------------------------------------------------------------------------- */
MultipleRegression* multireg_fit(double* X, double* y, int n, int p) {
    /* X: n×p matrix in row-major (no intercept column, added internally)
     * y: n×1 vector
     * p: number of predictors
     */
    if (n <= p || p < 1) return NULL;

    int pp = p + 1;  /* plus intercept */
    MultipleRegression* mr = (MultipleRegression*)malloc(sizeof(MultipleRegression));
    if (!mr) return NULL;
    mr->beta = (double*)calloc(pp, sizeof(double));
    mr->p = p;
    mr->r_squared = 0.0;
    if (!mr->beta) { free(mr); return NULL; }

    /* Build X^T X (pp×pp) and X^T y (pp×1) */
    double* XtX = (double*)calloc(pp * pp, sizeof(double));
    double* Xty = (double*)calloc(pp, sizeof(double));
    if (!XtX || !Xty) {
        free(XtX); free(Xty); free(mr->beta); free(mr); return NULL;
    }

    for (int i = 0; i < n; i++) {
        double row[pp];
        row[0] = 1.0;
        for (int j = 0; j < p; j++) row[j + 1] = X[i * p + j];

        for (int r = 0; r < pp; r++) {
            Xty[r] += row[r] * y[i];
            for (int c = 0; c < pp; c++) {
                XtX[r * pp + c] += row[r] * row[c];
            }
        }
    }

    /* Solve XtX * beta = Xty via Gaussian elimination with partial pivoting */
    double* A = (double*)malloc(pp * pp * sizeof(double));
    double* b = (double*)malloc(pp * sizeof(double));
    if (!A || !b) { free(A); free(b); free(XtX); free(Xty); free(mr->beta); free(mr); return NULL; }
    memcpy(A, XtX, pp * pp * sizeof(double));
    memcpy(b, Xty, pp * sizeof(double));

    for (int col = 0; col < pp; col++) {
        /* Partial pivot */
        int pivot = col;
        double max_val = fabs(A[col * pp + col]);
        for (int row = col + 1; row < pp; row++) {
            double v = fabs(A[row * pp + col]);
            if (v > max_val) { max_val = v; pivot = row; }
        }
        if (max_val < 1e-15) {
            /* Singular — set to 0 */
            for (int r = 0; r < pp; r++) mr->beta[r] = 0.0;
            free(A); free(b); free(XtX); free(Xty);
            return mr;
        }

        if (pivot != col) {
            for (int c = 0; c < pp; c++) {
                double t = A[col * pp + c];
                A[col * pp + c] = A[pivot * pp + c];
                A[pivot * pp + c] = t;
            }
            double t = b[col]; b[col] = b[pivot]; b[pivot] = t;
        }

        /* Eliminate below */
        for (int row = col + 1; row < pp; row++) {
            double factor = A[row * pp + col] / A[col * pp + col];
            for (int c = col; c < pp; c++)
                A[row * pp + c] -= factor * A[col * pp + c];
            b[row] -= factor * b[col];
        }
    }

    /* Back substitution */
    for (int row = pp - 1; row >= 0; row--) {
        double sum = b[row];
        for (int c = row + 1; c < pp; c++)
            sum -= A[row * pp + c] * mr->beta[c];
        mr->beta[row] = sum / A[row * pp + row];
    }

    /* Compute R-squared */
    double ybar = mean(y, n);
    double ss_tot = 0.0, ss_res = 0.0;
    for (int i = 0; i < n; i++) {
        double pred = mr->beta[0];
        for (int j = 0; j < p; j++) pred += mr->beta[j + 1] * X[i * p + j];
        double res = y[i] - pred;
        ss_res += res * res;
        ss_tot += (y[i] - ybar) * (y[i] - ybar);
    }
    mr->r_squared = (ss_tot > 0.0) ? (1.0 - ss_res / ss_tot) : 0.0;

    free(A); free(b); free(XtX); free(Xty);
    return mr;
}

double multireg_predict(MultipleRegression* mr, double* x) {
    if (!mr || !mr->beta) return 0.0;
    double y = mr->beta[0];
    for (int j = 0; j < mr->p; j++) y += mr->beta[j + 1] * x[j];
    return y;
}

void multireg_free(MultipleRegression* mr) {
    if (mr) { free(mr->beta); free(mr); }
}

/* --------------------------------------------------------------------------
 * L5: Logistic Regression (binary, stochastic gradient descent)
 *
 * Model: P(Y=1|x) = σ(β^T x), σ(z) = 1/(1+e^{-z})
 * Loss: Cross-entropy = -[y log(p) + (1-y) log(1-p)]
 *
 * Gradient: β += η (y - p) x
 * Complexity: O(n·p·max_iter)
 * -------------------------------------------------------------------------- */
static double sigmoid(double z) {
    if (z > 20.0) return 1.0;
    if (z < -20.0) return 0.0;
    return 1.0 / (1.0 + exp(-z));
}

LogisticRegression* logreg_fit(double* X, int* y, int n, int p,
                                double lr, int max_iter) {
    if (n < 2 || p < 1) return NULL;

    LogisticRegression* logreg = (LogisticRegression*)malloc(sizeof(LogisticRegression));
    if (!logreg) return NULL;
    logreg->beta = (double*)calloc(p + 1, sizeof(double));
    logreg->p = p;
    logreg->converged = false;
    if (!logreg->beta) { free(logreg); return NULL; }

    /* Initialize β to small random values */
    for (int j = 0; j <= p; j++)
        logreg->beta[j] = 0.01 * ((double)rand() / RAND_MAX - 0.5);

    /* Gradient descent */
    for (int iter = 0; iter < max_iter; iter++) {
        double* grad = (double*)calloc(p + 1, sizeof(double));
        double total_loss = 0.0;
        if (!grad) break;

        for (int i = 0; i < n; i++) {
            double z = logreg->beta[0];  /* intercept */
            for (int j = 0; j < p; j++)
                z += logreg->beta[j + 1] * X[i * p + j];

            double prob = sigmoid(z);
            double err = y[i] - prob;

            grad[0] += err;
            for (int j = 0; j < p; j++)
                grad[j + 1] += err * X[i * p + j];

            if (prob > 0.0 && prob < 1.0)
                total_loss -= (y[i] * log(prob) + (1 - y[i]) * log(1.0 - prob));
        }

        /* Update */
        double grad_norm = 0.0;
        for (int j = 0; j <= p; j++) {
            logreg->beta[j] += lr * grad[j] / n;
            grad_norm += grad[j] * grad[j];
        }
        free(grad);

        if (sqrt(grad_norm) < 1e-6) {
            logreg->converged = true;
            break;
        }
    }
    return logreg;
}

double logreg_predict_prob(LogisticRegression* lr_model, double* x) {
    if (!lr_model || !lr_model->beta) return 0.5;
    double z = lr_model->beta[0];
    for (int j = 0; j < lr_model->p; j++)
        z += lr_model->beta[j + 1] * x[j];
    return sigmoid(z);
}

int logreg_predict(LogisticRegression* lr_model, double* x) {
    return logreg_predict_prob(lr_model, x) >= 0.5 ? 1 : 0;
}

void logreg_free(LogisticRegression* lr_model) {
    if (lr_model) { free(lr_model->beta); free(lr_model); }
}

/* --------------------------------------------------------------------------
 * L5: Polynomial Regression (via OLS)
 *
 * Model: y = β₀ + β₁x + β₂x² + ... + β_{d}x^{d}
 *
 * Reformulated as multiple linear regression with Vandermonde features:
 *   X = [1, x, x², ..., x^{d}]
 * Complexity: O(n·d² + d³)
 * -------------------------------------------------------------------------- */
PolynomialRegression* polyreg_fit(double* x, double* y, int n, int degree) {
    if (n <= degree || degree < 1) return NULL;

    PolynomialRegression* pr = (PolynomialRegression*)malloc(sizeof(PolynomialRegression));
    if (!pr) return NULL;
    pr->beta = (double*)calloc(degree + 1, sizeof(double));
    pr->degree = degree;
    pr->r_squared = 0.0;
    if (!pr->beta) { free(pr); return NULL; }

    /* Build Vandermonde matrix: X[i][j] = x_i^j for j=0,...,degree */
    double* X = (double*)malloc(n * (degree + 1) * sizeof(double));
    if (!X) { free(pr->beta); free(pr); return NULL; }
    for (int i = 0; i < n; i++) {
        X[i * (degree + 1) + 0] = 1.0;
        for (int j = 1; j <= degree; j++) {
            X[i * (degree + 1) + j] = X[i * (degree + 1) + (j - 1)] * x[i];
        }
    }

    /* Solve via MultipleRegression — skip column 0 (intercept) of X since
     * multireg_fit adds its own intercept column internally. */
    double* X_no_intercept = (double*)malloc(n * degree * sizeof(double));
    if (!X_no_intercept) { free(X); free(pr->beta); free(pr); return NULL; }
    for (int i = 0; i < n; i++) {
        for (int j = 1; j <= degree; j++) {
            X_no_intercept[i * degree + (j - 1)] = X[i * (degree + 1) + j];
        }
    }

    MultipleRegression* mr = multireg_fit(X_no_intercept, y, n, degree);
    if (mr) {
        memcpy(pr->beta, mr->beta, (degree + 1) * sizeof(double));
        pr->r_squared = mr->r_squared;
        multireg_free(mr);
    }
    free(X_no_intercept);

    free(X);
    return pr;
}

double polyreg_predict(PolynomialRegression* pr, double x) {
    if (!pr || !pr->beta) return 0.0;
    double result = pr->beta[0];
    double xpow = 1.0;
    for (int j = 1; j <= pr->degree; j++) {
        xpow *= x;
        result += pr->beta[j] * xpow;
    }
    return result;
}

void polyreg_free(PolynomialRegression* pr) {
    if (pr) { free(pr->beta); free(pr); }
}

/* --------------------------------------------------------------------------
 * L8: Ridge Regression (L2 Regularization)
 *
 * β̂_ridge = (X^T X + λI)^{-1} X^T y
 *
 * λ → 0: approaches OLS
 * λ → ∞: coefficients shrink toward 0
 *
 * Uses simple approach: add λ to diagonal of X^TX
 * Complexity: O(np² + p³)
 * -------------------------------------------------------------------------- */
RidgeRegression* ridge_fit(double* X, double* y, int n, int p, double lambda) {
    if (n <= p || p < 1) return NULL;

    int pp = p + 1;
    RidgeRegression* rr = (RidgeRegression*)malloc(sizeof(RidgeRegression));
    if (!rr) return NULL;
    rr->beta = (double*)calloc(pp, sizeof(double));
    rr->p = p;
    rr->lambda = lambda;
    rr->r_squared = 0.0;
    if (!rr->beta) { free(rr); return NULL; }

    double* XtX = (double*)calloc(pp * pp, sizeof(double));
    double* Xty = (double*)calloc(pp, sizeof(double));
    if (!XtX || !Xty) {
        free(XtX); free(Xty); free(rr->beta); free(rr); return NULL;
    }

    for (int i = 0; i < n; i++) {
        double row[pp];
        row[0] = 1.0;
        for (int j = 0; j < p; j++) row[j + 1] = X[i * p + j];

        for (int r = 0; r < pp; r++) {
            Xty[r] += row[r] * y[i];
            for (int c = 0; c < pp; c++) {
                XtX[r * pp + c] += row[r] * row[c];
            }
        }
    }

    /* Add λ to diagonal (except intercept → column 0) */
    for (int j = 1; j < pp; j++) {
        XtX[j * pp + j] += lambda;
    }

    /* Solve via Gaussian elimination */
    double* A = (double*)malloc(pp * pp * sizeof(double));
    double* b = (double*)malloc(pp * sizeof(double));
    if (!A || !b) { free(A); free(b); free(XtX); free(Xty); free(rr->beta); free(rr); return NULL; }
    memcpy(A, XtX, pp * pp * sizeof(double));
    memcpy(b, Xty, pp * sizeof(double));

    for (int col = 0; col < pp; col++) {
        int pivot = col;
        double max_val = fabs(A[col * pp + col]);
        for (int row = col + 1; row < pp; row++) {
            double v = fabs(A[row * pp + col]);
            if (v > max_val) { max_val = v; pivot = row; }
        }
        if (max_val < 1e-15) {
            for (int r = 0; r < pp; r++) rr->beta[r] = 0.0;
            free(A); free(b); free(XtX); free(Xty); return rr;
        }
        if (pivot != col) {
            for (int c = 0; c < pp; c++) { double t = A[col*pp+c]; A[col*pp+c] = A[pivot*pp+c]; A[pivot*pp+c] = t; }
            double t = b[col]; b[col] = b[pivot]; b[pivot] = t;
        }
        for (int row = col + 1; row < pp; row++) {
            double factor = A[row * pp + col] / A[col * pp + col];
            for (int c = col; c < pp; c++) A[row * pp + c] -= factor * A[col * pp + c];
            b[row] -= factor * b[col];
        }
    }
    for (int row = pp - 1; row >= 0; row--) {
        double sum = b[row];
        for (int c = row + 1; c < pp; c++) sum -= A[row * pp + c] * rr->beta[c];
        rr->beta[row] = sum / A[row * pp + row];
    }

    double ybar = mean(y, n);
    double ss_tot = 0.0, ss_res = 0.0;
    for (int i = 0; i < n; i++) {
        double pred = rr->beta[0];
        for (int j = 0; j < p; j++) pred += rr->beta[j + 1] * X[i * p + j];
        double res = y[i] - pred;
        ss_res += res * res;
        ss_tot += (y[i] - ybar) * (y[i] - ybar);
    }
    rr->r_squared = (ss_tot > 0.0) ? (1.0 - ss_res / ss_tot) : 0.0;

    free(A); free(b); free(XtX); free(Xty);
    return rr;
}

double ridge_predict(RidgeRegression* rr, double* x) {
    if (!rr || !rr->beta) return 0.0;
    double y = rr->beta[0];
    for (int j = 0; j < rr->p; j++) y += rr->beta[j + 1] * x[j];
    return y;
}

void ridge_free(RidgeRegression* rr) {
    if (rr) { free(rr->beta); free(rr); }
}
