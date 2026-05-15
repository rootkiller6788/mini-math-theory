#include "regression.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int main(void) {
    srand((unsigned)time(NULL));

    printf("=== Simple Linear Regression Demo ===\n\n");

    int n = 10;
    double x[10];
    double y[10];

    double true_beta0 = 1.0;
    double true_beta1 = 2.0;

    printf("Generating data: y = %.1f + %.1f * x + noise\n", true_beta0, true_beta1);
    printf("%-6s %-10s %-10s\n", "x", "y", "y_true");
    printf("-----------------------------\n");

    for (int i = 0; i < n; i++) {
        x[i] = i + 1.0;
        double noise = ((double)rand() / RAND_MAX - 0.5) * 2.0;
        y[i] = true_beta0 + true_beta1 * x[i] + noise;
        printf("%-6.1f %-10.4f %-10.4f\n", x[i], y[i], true_beta0 + true_beta1 * x[i]);
    }

    printf("\nFitting linear regression...\n\n");

    LinearRegression* lr = linreg_fit(x, y, n);
    if (!lr) {
        printf("Error: could not fit regression\n");
        return 1;
    }

    linreg_print(lr);

    printf("\nPredictions:\n");
    for (int i = 0; i < n; i++) {
        double pred = linreg_predict(lr, x[i]);
        printf("  x=%.1f, y=%.4f, y_pred=%.4f, resid=%.4f\n",
               x[i], y[i], pred, y[i] - pred);
    }

    double* resids = linreg_residuals(x, y, n, lr->beta0, lr->beta1);
    double ss_res = 0.0;
    for (int i = 0; i < n; i++) ss_res += resids[i] * resids[i];
    printf("\nSum of squared residuals: %.4f\n", ss_res);
    free(resids);

    linreg_free(lr);
    return 0;
}
