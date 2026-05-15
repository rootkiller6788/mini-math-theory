#include "svd.h"
#include "eigen.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPS 1e-10

void svd_power_method(const Matrix* A, Matrix** U, Vector** Sigma, Matrix** Vt,
                      int k, int max_iter) {
    int m = A->rows;
    int n = A->cols;
    int p = k < n ? k : n;

    Matrix* Umat = mat_create(m, p);
    Vector* Svec = vec_create(p);
    Matrix* Vtmat = mat_create(p, n);

    Matrix* At = mat_transpose(A);
    Matrix* AtA = mat_mul(At, A);
    mat_free(At);

    Vector* v = vec_create(n);
    for (int s = 0; s < p; s++) {
        for (int i = 0; i < n; i++)
            v->data[i] = 1.0;

        double sigma = eigen_power_iteration(AtA, v, max_iter);

        if (sigma < EPS) sigma = 0.0;
        sigma = sqrt(sigma > 0 ? sigma : 0.0);

        Svec->data[s] = sigma;

        if (sigma > EPS) {
            Vector* Av = mat_vec_mul(A, v);
            for (int i = 0; i < m; i++)
                Umat->data[i * p + s] = Av->data[i] / sigma;
            vec_free(Av);
        } else {
            for (int i = 0; i < m; i++)
                Umat->data[i * p + s] = 0.0;
        }

        for (int i = 0; i < n; i++)
            Vtmat->data[s * n + i] = v->data[i];

        if (s < p - 1) {
            double sig2 = sigma * sigma;
            if (sig2 > EPS) {
                Vector* vcopy = vec_clone(v);
                eigen_deflate(AtA, sig2, vcopy);
                vec_free(vcopy);
            }
        }
    }

    vec_free(v);
    mat_free(AtA);

    *U = Umat;
    *Sigma = Svec;
    *Vt = Vtmat;
}

Matrix* svd_low_rank_approx(const Matrix* A, int k) {
    Matrix* U = NULL;
    Vector* Sigma = NULL;
    Matrix* Vt = NULL;
    svd_power_method(A, &U, &Sigma, &Vt, k, 1000);
    Matrix* result = svd_reconstruct(U, Sigma, Vt, k);
    mat_free(U);
    vec_free(Sigma);
    mat_free(Vt);
    return result;
}

Matrix* svd_reconstruct(const Matrix* U, const Vector* Sigma, const Matrix* Vt, int k) {
    int m = U->rows;
    int n = Vt->cols;

    Matrix* result = mat_create(m, n);

    for (int r = 0; r < k; r++) {
        double sigma = Sigma->data[r];
        if (fabs(sigma) < EPS) continue;

        for (int i = 0; i < m; i++) {
            double ui = U->data[i * U->cols + r];
            for (int j = 0; j < n; j++) {
                result->data[i * n + j] += sigma * ui * Vt->data[r * n + j];
            }
        }
    }
    return result;
}
