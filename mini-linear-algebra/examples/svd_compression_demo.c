#include "svd.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("=== SVD Compression Demo: 5x4 Matrix ===\n\n");

    double data[] = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 8, 7, 6,
        5, 4, 3, 2,
        1, 0, 1, 0
    };
    Matrix* A = mat_from_array(5, 4, data);

    printf("Original matrix A (5x4):\n");
    mat_print(A);

    int k = 3;
    printf("\nComputing SVD with k=%d...\n", k);

    Matrix* U = NULL;
    Vector* Sigma = NULL;
    Matrix* Vt = NULL;
    svd_power_method(A, &U, &Sigma, &Vt, k, 1000);

    printf("\nSingular values:\n");
    vec_print(Sigma);

    printf("\nU matrix (%dx%d):\n", U->rows, U->cols);
    mat_print(U);

    printf("\nV^T matrix (%dx%d):\n", Vt->rows, Vt->cols);
    mat_print(Vt);

    printf("\n=== Low-Rank Reconstructions ===\n");
    printf("Frobenius norm of original: %.6f\n", mat_frobenius_norm(A));

    for (int r = 1; r <= k; r++) {
        Matrix* approx = svd_reconstruct(U, Sigma, Vt, r);
        printf("\nRank-%d reconstruction:\n", r);
        mat_print(approx);

        Matrix* err = mat_sub(A, approx);
        double err_norm = mat_frobenius_norm(err);
        printf("Frobenius norm error: %.6f\n", err_norm);
        printf("Relative error: %.4f%%\n", err_norm / mat_frobenius_norm(A) * 100);
        mat_free(approx);
        mat_free(err);
    }

    printf("\nCompression summary:\n");
    printf("Original storage: %d doubles\n", A->rows * A->cols);
    for (int r = 1; r <= k; r++) {
        int svd_storage = U->rows * r + r + r * Vt->cols;
        printf("Rank-%d SVD storage: %d doubles (%.1f%% of original)\n",
               r, svd_storage,
               100.0 * svd_storage / (A->rows * A->cols));
    }

    mat_free(A);
    mat_free(U);
    vec_free(Sigma);
    mat_free(Vt);

    return 0;
}
