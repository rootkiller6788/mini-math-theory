#include "decompositions.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPS 1e-10

void decompose_qr(const Matrix* A, Matrix** Q, Matrix** R) {
    int m = A->rows;
    int n = A->cols;

    Matrix* Qmat = mat_create(m, n);
    Matrix* Rmat = mat_create(n, n);

    double* v = (double*)malloc(m * sizeof(double));

    for (int j = 0; j < n; j++) {
        for (int i = 0; i < m; i++)
            v[i] = A->data[i * n + j];

        for (int k = 0; k < j; k++) {
            double dot = 0.0;
            for (int i = 0; i < m; i++)
                dot += Qmat->data[i * n + k] * v[i];
            Rmat->data[k * n + j] = dot;
            for (int i = 0; i < m; i++)
                v[i] -= dot * Qmat->data[i * n + k];
        }

        double norm = 0.0;
        for (int i = 0; i < m; i++)
            norm += v[i] * v[i];
        norm = sqrt(norm);

        if (norm > EPS) {
            Rmat->data[j * n + j] = norm;
            for (int i = 0; i < m; i++)
                Qmat->data[i * n + j] = v[i] / norm;
        } else {
            Rmat->data[j * n + j] = 0.0;
        }
    }

    free(v);
    *Q = Qmat;
    *R = Rmat;
}

void decompose_qr_mgs(const Matrix* A, Matrix** Q, Matrix** R) {
    int m = A->rows;
    int n = A->cols;

    Matrix* Qmat = mat_create(m, n);
    Matrix* Rmat = mat_create(n, n);

    double* v = (double*)malloc(m * sizeof(double));

    for (int j = 0; j < n; j++) {
        for (int i = 0; i < m; i++)
            v[i] = A->data[i * n + j];

        for (int k = 0; k < j; k++) {
            double dot = 0.0;
            for (int i = 0; i < m; i++)
                dot += Qmat->data[i * n + k] * v[i];
            Rmat->data[k * n + j] += dot;
            for (int i = 0; i < m; i++)
                v[i] -= dot * Qmat->data[i * n + k];
        }

        double norm = 0.0;
        for (int i = 0; i < m; i++)
            norm += v[i] * v[i];
        norm = sqrt(norm);

        if (norm > EPS) {
            Rmat->data[j * n + j] = norm;
            for (int i = 0; i < m; i++)
                Qmat->data[i * n + j] = v[i] / norm;
        }
    }

    free(v);
    *Q = Qmat;
    *R = Rmat;
}

Matrix* decompose_cholesky(const Matrix* A) {
    int n = A->rows;
    Matrix* L = mat_create(n, n);

    for (int j = 0; j < n; j++) {
        double sum = 0.0;
        for (int k = 0; k < j; k++)
            sum += L->data[j * n + k] * L->data[j * n + k];
        double val = A->data[j * n + j] - sum;
        if (val < -EPS) return NULL;
        if (val < EPS) val = 0.0;
        L->data[j * n + j] = sqrt(val);

        for (int i = j + 1; i < n; i++) {
            sum = 0.0;
            for (int k = 0; k < j; k++)
                sum += L->data[i * n + k] * L->data[j * n + k];
            if (fabs(L->data[j * n + j]) > EPS)
                L->data[i * n + j] = (A->data[i * n + j] - sum) / L->data[j * n + j];
        }
    }
    return L;
}

Vector* eigen_qr_algorithm(Matrix* A, int max_iter) {
    int n = A->rows;
    Matrix* Ak = mat_clone(A);
    Vector* eig = vec_create(n);

    for (int iter = 0; iter < max_iter; iter++) {
        Matrix* Q = NULL;
        Matrix* R = NULL;
        decompose_qr(Ak, &Q, &R);
        Matrix* next = mat_mul(R, Q);
        mat_free(Ak);
        mat_free(Q);
        mat_free(R);
        Ak = next;

        int converged = 1;
        for (int i = 0; i < n - 1; i++) {
            double sub = 0.0;
            for (int j = i + 1; j < n; j++)
                sub += Ak->data[j * n + i] * Ak->data[j * n + i];
            if (sqrt(sub) > EPS * 100) {
                converged = 0;
                break;
            }
        }
        if (converged) break;
    }

    for (int i = 0; i < n; i++)
        eig->data[i] = Ak->data[i * n + i];

    mat_free(Ak);
    return eig;
}

Vector* solve_least_squares(const Matrix* A, const Vector* b) {
    Matrix* Q = NULL;
    Matrix* R = NULL;
    decompose_qr(A, &Q, &R);

    int m = A->rows;
    int n = A->cols;

    Vector* Qtb = vec_create(n);
    for (int j = 0; j < n; j++) {
        double sum = 0.0;
        for (int i = 0; i < m; i++)
            sum += Q->data[i * n + j] * b->data[i];
        Qtb->data[j] = sum;
    }

    Vector* x = vec_create(n);
    for (int i = n - 1; i >= 0; i--) {
        double sum = Qtb->data[i];
        for (int j = i + 1; j < n; j++)
            sum -= R->data[i * n + j] * x->data[j];
        if (fabs(R->data[i * n + i]) > EPS)
            x->data[i] = sum / R->data[i * n + i];
        else
            x->data[i] = 0.0;
    }

    vec_free(Qtb);
    mat_free(Q);
    mat_free(R);
    return x;
}
