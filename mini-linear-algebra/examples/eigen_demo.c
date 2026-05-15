#include "eigen.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("=== Power Iteration ===\n");
    printf("A = [[2, -1], [-1, 2]]\n\n");

    double A_data[] = {2, -1, -1, 2};
    Matrix* A = mat_from_array(2, 2, A_data);

    Vector* v = vec_create(2);
    double lambda = eigen_power_iteration(A, v, 1000);

    printf("Dominant eigenvalue: %.6f (expected: 3)\n", lambda);
    printf("Dominant eigenvector:\n");
    vec_print(v);
    printf("(Expected: approx [0.707, -0.707])\n\n");

    printf("Verification A*v = lambda*v:\n");
    Vector* Av = mat_vec_mul(A, v);
    Vector* lv = vec_scale(v, lambda);
    Vector* diff = vec_sub(Av, lv);
    printf("||A*v - lambda*v|| = %.2e\n", vec_norm(diff));
    vec_free(Av);
    vec_free(lv);
    vec_free(diff);

    printf("\n=== QR Algorithm: All Eigenvalues ===\n");

    double A3_data[] = {4, 1, 1, 1, 3, 1, 1, 1, 2};
    Matrix* A3 = mat_from_array(3, 3, A3_data);

    printf("Matrix A:\n");
    mat_print(A3);

    Vector* eigvals = eigen_all(A3, 1000);
    printf("\nEigenvalues:\n");
    vec_print(eigvals);

    printf("\nVerification for each eigenvector (via power iteration after deflation):\n");
    Matrix* A_copy = mat_clone(A3);
    for (int i = 0; i < eigvals->size; i++) {
        Vector* ev = vec_create(3);
        double lambda_val = eigen_power_iteration(A_copy, ev, 1000);
        printf("  lambda_%d = %.6f\n", i + 1, lambda_val);
        printf("  v_%d:\n", i + 1);
        vec_print(ev);
        printf("  ||A*v - lambda*v|| = ");
        Vector* Av_i = mat_vec_mul(A3, ev);
        Vector* lv_i = vec_scale(ev, lambda_val);
        Vector* diff_i = vec_sub(Av_i, lv_i);
        printf("%.2e\n", vec_norm(diff_i));
        vec_free(Av_i);
        vec_free(lv_i);
        vec_free(diff_i);

        if (i < eigvals->size - 1)
            eigen_deflate(A_copy, lambda_val, ev);
        vec_free(ev);
    }

    mat_free(A_copy);
    vec_free(eigvals);
    mat_free(A3);
    vec_free(v);
    mat_free(A);

    return 0;
}
