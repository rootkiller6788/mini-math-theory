#include "eigen.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("=== PCA Demo: 2D Correlated Data ===\n\n");

    int n_samples = 6;
    double raw_data[] = {
        2.5, 2.4,
        0.5, 0.7,
        2.2, 2.9,
        1.9, 2.2,
        3.1, 3.0,
        2.3, 2.7
    };
    int n_features = 2;

    printf("Original data (%d samples x %d features):\n", n_samples, n_features);
    for (int i = 0; i < n_samples; i++) {
        printf("  [%6.2f %6.2f]\n", raw_data[i * 2], raw_data[i * 2 + 1]);
    }

    double mean[2] = {0, 0};
    for (int i = 0; i < n_samples; i++) {
        mean[0] += raw_data[i * 2];
        mean[1] += raw_data[i * 2 + 1];
    }
    mean[0] /= n_samples;
    mean[1] /= n_samples;
    printf("\nMean: [%.4f, %.4f]\n", mean[0], mean[1]);

    double centered_data[12];
    for (int i = 0; i < n_samples; i++) {
        centered_data[i * 2] = raw_data[i * 2] - mean[0];
        centered_data[i * 2 + 1] = raw_data[i * 2 + 1] - mean[1];
    }

    double cov[4] = {0, 0, 0, 0};
    for (int i = 0; i < n_samples; i++) {
        for (int r = 0; r < 2; r++) {
            for (int c = 0; c < 2; c++) {
                cov[r * 2 + c] += centered_data[i * 2 + r] * centered_data[i * 2 + c];
            }
        }
    }
    for (int i = 0; i < 4; i++)
        cov[i] /= (n_samples - 1);

    printf("\nCovariance matrix:\n");
    printf("  [%8.4f %8.4f]\n", cov[0], cov[1]);
    printf("  [%8.4f %8.4f]\n", cov[2], cov[3]);

    Matrix* cov_mat = mat_from_array(2, 2, cov);

    Vector* eigvals = eigen_all(cov_mat, 1000);
    printf("\nEigenvalues: ");
    vec_print(eigvals);

    Matrix* cov_copy = mat_clone(cov_mat);
    printf("\nPrincipal Components:\n");
    for (int i = 0; i < 2; i++) {
        Vector* pc = vec_create(2);
        double lambda_i = eigen_power_iteration(cov_copy, pc, 1000);
        printf("  PC%d (lambda=%.6f):\n", i + 1, lambda_i);
        vec_print(pc);

        if (i == 0) {
            printf("\nProjection onto PC1:\n");
            for (int j = 0; j < n_samples; j++) {
                double proj = centered_data[j * 2] * pc->data[0] + centered_data[j * 2 + 1] * pc->data[1];
                printf("  sample %d: %.4f\n", j + 1, proj);
            }
        }

        if (i < 1)
            eigen_deflate(cov_copy, lambda_i, pc);
        vec_free(pc);
    }

    mat_free(cov_copy);
    mat_free(cov_mat);
    vec_free(eigvals);

    return 0;
}
