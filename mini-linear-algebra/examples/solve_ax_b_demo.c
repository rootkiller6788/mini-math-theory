#include "linalg.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("=== Solve Ax = b: 2x2 system ===\n");
    printf("A = [[3,2],[1,2]], b = [7,5]^T\n\n");

    double A_data[] = {3, 2, 1, 2};
    Matrix* A = mat_from_array(2, 2, A_data);
    double b_data[] = {7, 5};
    Vector* b = vec_from_array(b_data, 2);

    Vector* x = solve_ax_b(A, b);
    if (x) {
        printf("Solution x:\n");
        vec_print(x);
        printf("(Expected: [1, 2])\n\n");

        printf("Verification A*x:\n");
        Vector* Ax = mat_vec_mul(A, x);
        vec_print(Ax);
        printf("(Should equal b)\n\n");
        vec_free(Ax);
        vec_free(x);
    } else {
        printf("No solution found.\n\n");
    }

    printf("=== Solve Ax = b: 3x3 system via LU ===\n");
    double A3_data[] = {4, -2, 1, -2, 4, -2, 1, -2, 4};
    Matrix* A3 = mat_from_array(3, 3, A3_data);
    double b3_data[] = {3, -1, 3};
    Vector* b3 = vec_from_array(b3_data, 3);

    int* pivot = (int*)malloc(3 * sizeof(int));
    Matrix* LU = mat_clone(A3);
    bool ok = decompose_lu(LU, pivot);

    if (ok) {
        printf("LU decomposition successful.\n");
        printf("LU combined matrix:\n");
        mat_print(LU);

        Vector* x3 = solve_lu(LU, pivot, b3);
        printf("Solution x:\n");
        vec_print(x3);

        printf("Verification A*x - b:\n");
        Vector* Ax3 = mat_vec_mul(A3, x3);
        Vector* diff = vec_sub(Ax3, b3);
        printf("Residual norm: %.2e\n", vec_norm(diff));
        vec_free(x3);
        vec_free(Ax3);
        vec_free(diff);
    } else {
        printf("LU decomposition failed.\n");
    }

    mat_free(A);
    mat_free(A3);
    mat_free(LU);
    vec_free(b);
    vec_free(b3);
    free(pivot);

    return 0;
}
