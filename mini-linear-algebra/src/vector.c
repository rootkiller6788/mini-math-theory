#include "vector.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPS 1e-10

Vector* vec_create(int size) {
    Vector* v = (Vector*)malloc(sizeof(Vector));
    v->size = size;
    v->data = (double*)calloc(size, sizeof(double));
    return v;
}

Vector* vec_from_array(const double* arr, int size) {
    Vector* v = vec_create(size);
    memcpy(v->data, arr, size * sizeof(double));
    return v;
}

void vec_free(Vector* v) {
    if (v) {
        free(v->data);
        free(v);
    }
}

Vector* vec_clone(const Vector* v) {
    return vec_from_array(v->data, v->size);
}

double vec_dot(const Vector* a, const Vector* b) {
    double sum = 0.0;
    for (int i = 0; i < a->size; i++)
        sum += a->data[i] * b->data[i];
    return sum;
}

double vec_norm(const Vector* v) {
    return sqrt(vec_dot(v, v));
}

double vec_norm_p(const Vector* v, double p) {
    double sum = 0.0;
    for (int i = 0; i < v->size; i++)
        sum += pow(fabs(v->data[i]), p);
    return pow(sum, 1.0 / p);
}

Vector* vec_add(const Vector* a, const Vector* b) {
    Vector* r = vec_create(a->size);
    for (int i = 0; i < a->size; i++)
        r->data[i] = a->data[i] + b->data[i];
    return r;
}

Vector* vec_sub(const Vector* a, const Vector* b) {
    Vector* r = vec_create(a->size);
    for (int i = 0; i < a->size; i++)
        r->data[i] = a->data[i] - b->data[i];
    return r;
}

Vector* vec_scale(const Vector* v, double s) {
    Vector* r = vec_create(v->size);
    for (int i = 0; i < v->size; i++)
        r->data[i] = v->data[i] * s;
    return r;
}

Vector* vec_normalize(const Vector* v) {
    double n = vec_norm(v);
    if (n < EPS) return vec_clone(v);
    return vec_scale(v, 1.0 / n);
}

Vector* vec_cross3(const Vector* a, const Vector* b) {
    Vector* r = vec_create(3);
    r->data[0] = a->data[1] * b->data[2] - a->data[2] * b->data[1];
    r->data[1] = a->data[2] * b->data[0] - a->data[0] * b->data[2];
    r->data[2] = a->data[0] * b->data[1] - a->data[1] * b->data[0];
    return r;
}

void vec_print(const Vector* v) {
    printf("[");
    for (int i = 0; i < v->size; i++) {
        printf("%8.4f", v->data[i]);
        if (i < v->size - 1) printf(" ");
    }
    printf(" ]\n");
}

double vec_get(const Vector* v, int i) {
    return v->data[i];
}

void vec_set(Vector* v, int i, double val) {
    v->data[i] = val;
}

double vec_distance(const Vector* a, const Vector* b) {
    Vector* d = vec_sub(a, b);
    double dist = vec_norm(d);
    vec_free(d);
    return dist;
}

double vec_angle(const Vector* a, const Vector* b) {
    double dot = vec_dot(a, b);
    double na = vec_norm(a);
    double nb = vec_norm(b);
    if (na < EPS || nb < EPS) return 0.0;
    double cos_theta = dot / (na * nb);
    if (cos_theta > 1.0) cos_theta = 1.0;
    if (cos_theta < -1.0) cos_theta = -1.0;
    return acos(cos_theta);
}
