#include "matrix.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPS 1e-10

static void lu_decompose(Matrix* A, int* pivot, int* sign);

Matrix* mat_create(int rows, int cols) {
    Matrix* m = (Matrix*)malloc(sizeof(Matrix));
    m->rows = rows;
    m->cols = cols;
    m->data = (double*)calloc(rows * cols, sizeof(double));
    return m;
}

Matrix* mat_create_identity(int n) {
    Matrix* m = mat_create(n, n);
    for (int i = 0; i < n; i++)
        m->data[i * n + i] = 1.0;
    return m;
}

Matrix* mat_create_zeros(int rows, int cols) {
    return mat_create(rows, cols);
}

Matrix* mat_create_ones(int rows, int cols) {
    Matrix* m = mat_create(rows, cols);
    for (int i = 0; i < rows * cols; i++)
        m->data[i] = 1.0;
    return m;
}

Matrix* mat_from_array(int rows, int cols, const double* data) {
    Matrix* m = mat_create(rows, cols);
    memcpy(m->data, data, rows * cols * sizeof(double));
    return m;
}

void mat_free(Matrix* m) {
    if (m) {
        free(m->data);
        free(m);
    }
}

Matrix* mat_clone(const Matrix* m) {
    return mat_from_array(m->rows, m->cols, m->data);
}

double mat_get(const Matrix* m, int r, int c) {
    return m->data[r * m->cols + c];
}

void mat_set(Matrix* m, int r, int c, double val) {
    m->data[r * m->cols + c] = val;
}

Matrix* mat_add(const Matrix* a, const Matrix* b) {
    Matrix* r = mat_create(a->rows, a->cols);
    for (int i = 0; i < a->rows * a->cols; i++)
        r->data[i] = a->data[i] + b->data[i];
    return r;
}

Matrix* mat_sub(const Matrix* a, const Matrix* b) {
    Matrix* r = mat_create(a->rows, a->cols);
    for (int i = 0; i < a->rows * a->cols; i++)
        r->data[i] = a->data[i] - b->data[i];
    return r;
}

Matrix* mat_scale(const Matrix* m, double s) {
    Matrix* r = mat_create(m->rows, m->cols);
    for (int i = 0; i < m->rows * m->cols; i++)
        r->data[i] = m->data[i] * s;
    return r;
}

Matrix* mat_mul(const Matrix* a, const Matrix* b) {
    Matrix* r = mat_create(a->rows, b->cols);
    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < b->cols; j++) {
            double sum = 0.0;
            for (int k = 0; k < a->cols; k++)
                sum += a->data[i * a->cols + k] * b->data[k * b->cols + j];
            r->data[i * b->cols + j] = sum;
        }
    }
    return r;
}

Vector* mat_vec_mul(const Matrix* m, const Vector* v) {
    Vector* r = vec_create(m->rows);
    for (int i = 0; i < m->rows; i++) {
        double sum = 0.0;
        for (int j = 0; j < m->cols; j++)
            sum += m->data[i * m->cols + j] * v->data[j];
        r->data[i] = sum;
    }
    return r;
}

Matrix* mat_transpose(const Matrix* m) {
    Matrix* r = mat_create(m->cols, m->rows);
    for (int i = 0; i < m->rows; i++)
        for (int j = 0; j < m->cols; j++)
            r->data[j * r->cols + i] = m->data[i * m->cols + j];
    return r;
}

static void lu_decompose(Matrix* A, int* pivot, int* sign) {
    int n = A->rows;
    *sign = 1;
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
        if (max_val < EPS) continue;

        if (max_row != k) {
            for (int j = 0; j < n; j++) {
                double tmp = A->data[k * n + j];
                A->data[k * n + j] = A->data[max_row * n + j];
                A->data[max_row * n + j] = tmp;
            }
            int tmp = pivot[k];
            pivot[k] = pivot[max_row];
            pivot[max_row] = tmp;
            *sign = -*sign;
        }

        double pivot_val = A->data[k * n + k];
        if (fabs(pivot_val) < EPS) continue;

        for (int i = k + 1; i < n; i++) {
            double factor = A->data[i * n + k] / pivot_val;
            A->data[i * n + k] = factor;
            for (int j = k + 1; j < n; j++)
                A->data[i * n + j] -= factor * A->data[k * n + j];
        }
    }
}

double mat_determinant(Matrix* m) {
    if (m->rows != m->cols) return 0.0;
    Matrix* A = mat_clone(m);
    int n = A->rows;
    int* pivot = (int*)malloc(n * sizeof(int));
    int sign;
    lu_decompose(A, pivot, &sign);
    double det = sign;
    for (int i = 0; i < n; i++)
        det *= A->data[i * n + i];
    mat_free(A);
    free(pivot);
    return det;
}

Matrix* mat_inverse(Matrix* m) {
    if (m->rows != m->cols) return NULL;
    int n = m->rows;

    Matrix* LU = mat_clone(m);
    int* pivot = (int*)malloc(n * sizeof(int));
    int sign;
    lu_decompose(LU, pivot, &sign);

    Matrix* inv = mat_create(n, n);

    double* b = (double*)calloc(n, sizeof(double));
    for (int col = 0; col < n; col++) {
        for (int i = 0; i < n; i++)
            b[i] = (pivot[i] == col) ? 1.0 : 0.0;

        double* x = (double*)malloc(n * sizeof(double));
        for (int i = 0; i < n; i++)
            x[i] = b[i];

        for (int i = 0; i < n; i++) {
            int pi = pivot[i];
            double sum = b[pi];
            b[pi] = b[i];
            if (i > 0) {
                for (int j = 0; j < i; j++)
                    sum -= LU->data[i * n + j] * b[pivot[j]];
            }
            b[i] = sum;
        }

        for (int i = n - 1; i >= 0; i--) {
            double sum = b[i];
            for (int j = i + 1; j < n; j++)
                sum -= LU->data[i * n + j] * b[j];
            if (fabs(LU->data[i * n + i]) > EPS)
                b[i] = sum / LU->data[i * n + i];
        }

        for (int i = 0; i < n; i++)
            inv->data[i * n + col] = b[i];

        free(x);
    }
    free(b);
    mat_free(LU);
    free(pivot);

    return inv;
}

double mat_trace(const Matrix* m) {
    double t = 0.0;
    int n = m->rows < m->cols ? m->rows : m->cols;
    for (int i = 0; i < n; i++)
        t += m->data[i * m->cols + i];
    return t;
}

double mat_frobenius_norm(const Matrix* m) {
    double sum = 0.0;
    for (int i = 0; i < m->rows * m->cols; i++)
        sum += m->data[i] * m->data[i];
    return sqrt(sum);
}

double mat_condition_number(const Matrix* m) {
    double norm_a = mat_frobenius_norm(m);
    Matrix* inv = mat_inverse((Matrix*)m);
    if (!inv) return INFINITY;
    double norm_a_inv = mat_frobenius_norm(inv);
    mat_free(inv);
    return norm_a * norm_a_inv;
}

void mat_print(const Matrix* m) {
    for (int i = 0; i < m->rows; i++) {
        printf("[");
        for (int j = 0; j < m->cols; j++) {
            printf("%8.4f", m->data[i * m->cols + j]);
            if (j < m->cols - 1) printf(" ");
        }
        printf(" ]\n");
    }
}

Matrix* mat_get_row(const Matrix* m, int r) {
    Matrix* row = mat_create(1, m->cols);
    for (int j = 0; j < m->cols; j++)
        row->data[j] = m->data[r * m->cols + j];
    return row;
}

Matrix* mat_get_column(const Matrix* m, int c) {
    Matrix* col = mat_create(m->rows, 1);
    for (int i = 0; i < m->rows; i++)
        col->data[i] = m->data[i * m->cols + c];
    return col;
}
