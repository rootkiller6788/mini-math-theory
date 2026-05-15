#include "tensor.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPS 1e-10

Tensor3D* tensor3d_create(int d1, int d2, int d3) {
    Tensor3D* t = (Tensor3D*)malloc(sizeof(Tensor3D));
    t->d1 = d1;
    t->d2 = d2;
    t->d3 = d3;
    t->data = (double*)calloc(d1 * d2 * d3, sizeof(double));
    return t;
}

void tensor3d_free(Tensor3D* t) {
    if (t) {
        free(t->data);
        free(t);
    }
}

double tensor3d_get(const Tensor3D* t, int i, int j, int k) {
    return t->data[(i * t->d2 + j) * t->d3 + k];
}

void tensor3d_set(Tensor3D* t, int i, int j, int k, double val) {
    t->data[(i * t->d2 + j) * t->d3 + k] = val;
}

Tensor3D* tensor3d_add(const Tensor3D* a, const Tensor3D* b) {
    Tensor3D* r = tensor3d_create(a->d1, a->d2, a->d3);
    int total = a->d1 * a->d2 * a->d3;
    for (int i = 0; i < total; i++)
        r->data[i] = a->data[i] + b->data[i];
    return r;
}

Tensor3D* tensor3d_scale(const Tensor3D* t, double s) {
    Tensor3D* r = tensor3d_create(t->d1, t->d2, t->d3);
    int total = t->d1 * t->d2 * t->d3;
    for (int i = 0; i < total; i++)
        r->data[i] = t->data[i] * s;
    return r;
}

Matrix* tensor3d_unfold(const Tensor3D* t, int mode) {
    if (mode == 0) {
        Matrix* m = mat_create(t->d1, t->d2 * t->d3);
        for (int i = 0; i < t->d1; i++)
            for (int j = 0; j < t->d2; j++)
                for (int k = 0; k < t->d3; k++)
                    m->data[i * m->cols + j * t->d3 + k] = tensor3d_get(t, i, j, k);
        return m;
    } else if (mode == 1) {
        Matrix* m = mat_create(t->d2, t->d1 * t->d3);
        for (int j = 0; j < t->d2; j++)
            for (int i = 0; i < t->d1; i++)
                for (int k = 0; k < t->d3; k++)
                    m->data[j * m->cols + i * t->d3 + k] = tensor3d_get(t, i, j, k);
        return m;
    } else {
        Matrix* m = mat_create(t->d3, t->d1 * t->d2);
        for (int k = 0; k < t->d3; k++)
            for (int i = 0; i < t->d1; i++)
                for (int j = 0; j < t->d2; j++)
                    m->data[k * m->cols + i * t->d2 + j] = tensor3d_get(t, i, j, k);
        return m;
    }
}

Tensor3D* tensor3d_fold(const Matrix* m, int d1, int d2, int d3, int mode) {
    Tensor3D* t = tensor3d_create(d1, d2, d3);
    if (mode == 0) {
        for (int i = 0; i < d1; i++)
            for (int j = 0; j < d2; j++)
                for (int k = 0; k < d3; k++)
                    tensor3d_set(t, i, j, k, m->data[i * m->cols + j * d3 + k]);
    } else if (mode == 1) {
        for (int j = 0; j < d2; j++)
            for (int i = 0; i < d1; i++)
                for (int k = 0; k < d3; k++)
                    tensor3d_set(t, i, j, k, m->data[j * m->cols + i * d3 + k]);
    } else {
        for (int k = 0; k < d3; k++)
            for (int i = 0; i < d1; i++)
                for (int j = 0; j < d2; j++)
                    tensor3d_set(t, i, j, k, m->data[k * m->cols + i * d2 + j]);
    }
    return t;
}

double tensor3d_frobenius_norm(const Tensor3D* t) {
    double sum = 0.0;
    int total = t->d1 * t->d2 * t->d3;
    for (int i = 0; i < total; i++)
        sum += t->data[i] * t->data[i];
    return sqrt(sum);
}

void tensor3d_print(const Tensor3D* t) {
    printf("Tensor3D (%d x %d x %d):\n", t->d1, t->d2, t->d3);
    for (int i = 0; i < t->d1; i++) {
        printf("  Slice %d:\n", i);
        for (int j = 0; j < t->d2; j++) {
            printf("    [");
            for (int k = 0; k < t->d3; k++) {
                printf("%8.4f", tensor3d_get(t, i, j, k));
                if (k < t->d3 - 1) printf(" ");
            }
            printf(" ]\n");
        }
    }
}
