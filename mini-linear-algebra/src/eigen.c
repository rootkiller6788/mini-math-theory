#include "eigen.h"
#include "decompositions.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPS 1e-10

double eigen_power_iteration(const Matrix* A, Vector* eigenvector, int max_iter) {
    int n = A->rows;

    for (int i = 0; i < n; i++)
        eigenvector->data[i] = 1.0;

    double lambda = 0.0;
    double lambda_old;

    for (int iter = 0; iter < max_iter; iter++) {
        Vector* y = mat_vec_mul(A, eigenvector);

        double norm = 0.0;
        for (int i = 0; i < n; i++)
            norm += y->data[i] * y->data[i];
        norm = sqrt(norm);

        if (norm < EPS) {
            vec_free(y);
            return 0.0;
        }

        for (int i = 0; i < n; i++)
            eigenvector->data[i] = y->data[i] / norm;

        lambda_old = lambda;
        Vector* Ax = mat_vec_mul(A, eigenvector);
        lambda = vec_dot(eigenvector, Ax);
        vec_free(Ax);

        vec_free(y);

        if (fabs(lambda - lambda_old) < EPS)
            break;
    }
    return lambda;
}

double eigen_inverse_power(const Matrix* A, Vector* eigenvector, double shift, int max_iter) {
    int n = A->rows;

    Matrix* Ashift = mat_clone(A);
    for (int i = 0; i < n; i++)
        Ashift->data[i * n + i] -= shift;

    Matrix* Ashift_inv = mat_inverse(Ashift);
    mat_free(Ashift);

    if (!Ashift_inv) return 0.0;

    for (int i = 0; i < n; i++)
        eigenvector->data[i] = 1.0;

    double lambda = 0.0;
    double lambda_old;

    for (int iter = 0; iter < max_iter; iter++) {
        Vector* y = mat_vec_mul(Ashift_inv, eigenvector);

        double norm = vec_norm(y);
        if (norm < EPS) { vec_free(y); break; }

        for (int i = 0; i < n; i++)
            eigenvector->data[i] = y->data[i] / norm;

        vec_free(y);

        lambda_old = lambda;
        Vector* Ax = mat_vec_mul(A, eigenvector);
        lambda = vec_dot(eigenvector, Ax);
        vec_free(Ax);

        if (fabs(lambda - lambda_old) < EPS)
            break;
    }

    mat_free(Ashift_inv);
    return lambda;
}

double eigen_rayleigh_quotient(const Matrix* A, const Vector* x) {
    Vector* Ax = mat_vec_mul(A, x);
    double num = vec_dot(x, Ax);
    double den = vec_dot(x, x);
    vec_free(Ax);
    if (fabs(den) < EPS) return 0.0;
    return num / den;
}

Vector* eigen_all(const Matrix* A, int max_iter) {
    int n = A->rows;
    Matrix* Ak = mat_clone(A);

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
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < i; j++) {
                if (fabs(Ak->data[i * n + j]) > EPS * 100) {
                    converged = 0;
                    break;
                }
            }
            if (!converged) break;
        }
        if (converged) break;
    }

    Vector* eig = vec_create(n);
    for (int i = 0; i < n; i++)
        eig->data[i] = Ak->data[i * n + i];

    mat_free(Ak);
    return eig;
}

void eigen_deflate(Matrix* A, double eigenvalue, const Vector* eigenvector) {
    int n = A->rows;

    Matrix* v = mat_create(n, 1);
    for (int i = 0; i < n; i++)
        v->data[i] = eigenvector->data[i];
    Matrix* vt = mat_create(1, n);
    for (int i = 0; i < n; i++)
        vt->data[i] = eigenvector->data[i];

    Matrix* outer = mat_mul(v, vt);
    Matrix* defl = mat_scale(outer, eigenvalue);

    for (int i = 0; i < n * A->cols; i++)
        A->data[i] -= defl->data[i];

    mat_free(v);
    mat_free(vt);
    mat_free(outer);
    mat_free(defl);
}
