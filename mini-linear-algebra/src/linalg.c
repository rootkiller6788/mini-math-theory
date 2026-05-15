#include "linalg.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPS 1e-10

Vector* solve_ax_b(Matrix* A, Vector* b) {
    int n = A->rows;
    int* pivot = (int*)malloc(n * sizeof(int));
    Matrix* LU = mat_clone(A);
    bool ok = decompose_lu(LU, pivot);
    if (!ok) {
        mat_free(LU);
        free(pivot);
        return NULL;
    }
    Vector* x = solve_lu(LU, pivot, b);
    mat_free(LU);
    free(pivot);
    return x;
}

bool decompose_lu(Matrix* A, int* pivot) {
    int n = A->rows;
    for (int i = 0; i < n; i++)
        pivot[i] = i;

    for (int k = 0; k < n; k++) {
        double max_val = 0.0;
        int max_row = k;
        for (int i = k; i < n; i++) {
            double val = fabs(A->data[i * n + k]);
            if (val > max_val) {
                max_val = val;
                max_row = i;
            }
        }
        if (max_val < EPS) return false;

        if (max_row != k) {
            for (int j = 0; j < n; j++) {
                double tmp = A->data[k * n + j];
                A->data[k * n + j] = A->data[max_row * n + j];
                A->data[max_row * n + j] = tmp;
            }
            int tmp = pivot[k];
            pivot[k] = pivot[max_row];
            pivot[max_row] = tmp;
        }

        double pivot_val = A->data[k * n + k];
        for (int i = k + 1; i < n; i++) {
            double factor = A->data[i * n + k] / pivot_val;
            A->data[i * n + k] = factor;
            for (int j = k + 1; j < n; j++)
                A->data[i * n + j] -= factor * A->data[k * n + j];
        }
    }
    return true;
}

Vector* solve_lu(const Matrix* LU, const int* pivot, const Vector* b) {
    int n = LU->rows;
    Vector* x = vec_create(n);

    double* y = (double*)malloc(n * sizeof(double));
    for (int i = 0; i < n; i++)
        y[i] = b->data[pivot[i]];

    for (int i = 0; i < n; i++) {
        double sum = y[i];
        for (int j = 0; j < i; j++)
            sum -= LU->data[i * n + j] * y[j];
        y[i] = sum;
    }

    for (int i = n - 1; i >= 0; i--) {
        double sum = y[i];
        for (int j = i + 1; j < n; j++)
            sum -= LU->data[i * n + j] * x->data[j];
        x->data[i] = sum / LU->data[i * n + i];
    }

    free(y);
    return x;
}

static int rank_from_lu(const Matrix* A_copy) {
    int n = A_copy->rows;
    int m = A_copy->cols;
    int min_dim = n < m ? n : m;
    int rank = 0;
    for (int i = 0; i < min_dim; i++)
        if (fabs(A_copy->data[i * n + i]) > EPS)
            rank++;
    return rank;
}

double mat_rank(Matrix* m) {
    int n = m->rows;
    int p = m->cols;
    Matrix* A = mat_clone(m);
    int* pivot = (int*)malloc(n * sizeof(int));
    bool ok = decompose_lu(A, pivot);
    double r = ok ? rank_from_lu(A) : 0.0;
    mat_free(A);
    free(pivot);
    return r;
}

Vector* mat_nullspace_vector(Matrix* m) {
    int rows = m->rows;
    int cols = m->cols;
    if (rows >= cols) {
        Matrix* A = mat_clone(m);
        int* pivot = (int*)malloc(rows * sizeof(int));
        bool ok = decompose_lu(A, pivot);
        if (!ok) {
            mat_free(A);
            free(pivot);
            return NULL;
        }

        int rank = rank_from_lu(A);
        if (rank == cols) {
            mat_free(A);
            free(pivot);
            return NULL;
        }

        Vector* v = vec_create(cols);
        int free_col = rank;

        v->data[free_col] = 1.0;

        for (int i = free_col - 1; i >= 0; i--) {
            double sum = 0.0;
            for (int j = i + 1; j < cols; j++)
                sum += A->data[i * rows + j] * v->data[j];
            if (fabs(A->data[i * rows + i]) > EPS)
                v->data[i] = -sum / A->data[i * rows + i];
        }

        mat_free(A);
        free(pivot);
        return v;
    } else {
        Matrix* At = mat_transpose(m);
        Matrix* AtA = mat_mul(At, m);
        Vector* v = mat_nullspace_vector(AtA);
        mat_free(At);
        mat_free(AtA);
        return v;
    }
}

void mat_col_space(Matrix* m) {
    int rows = m->rows;
    int cols = m->cols;
    Matrix* A = mat_clone(m);
    int* pivot = (int*)malloc(rows * sizeof(int));
    decompose_lu(A, pivot);

    int rank = rank_from_lu(A);

    printf("Column space basis (%d vectors):\n", rank);
    for (int r = 0; r < rank; r++) {
        printf("  v%d = [", r + 1);
        for (int i = 0; i < rows; i++) {
            int pi = pivot[i] >= 0 ? pivot[i] : i;
            printf("%8.4f", m->data[i * cols + pi]);
            if (i < rows - 1) printf(" ");
        }
        printf(" ]\n");
    }

    mat_free(A);
    free(pivot);
}

void mat_row_space(Matrix* m) {
    Matrix* T = mat_transpose(m);
    mat_col_space(T);
    mat_free(T);
}
