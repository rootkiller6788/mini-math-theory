#include "regression.h"
#include "inference.h"
#include "distribution.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

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
