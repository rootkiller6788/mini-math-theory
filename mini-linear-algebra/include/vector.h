#ifndef VECTOR_H
#define VECTOR_H

typedef struct { int size; double* data; } Vector;

Vector* vec_create(int size);
Vector* vec_from_array(const double* arr, int size);
void    vec_free(Vector* v);
Vector* vec_clone(const Vector* v);
double  vec_dot(const Vector* a, const Vector* b);
double  vec_norm(const Vector* v);
double  vec_norm_p(const Vector* v, double p);
Vector* vec_add(const Vector* a, const Vector* b);
Vector* vec_sub(const Vector* a, const Vector* b);
Vector* vec_scale(const Vector* v, double s);
Vector* vec_normalize(const Vector* v);
Vector* vec_cross3(const Vector* a, const Vector* b);
void    vec_print(const Vector* v);
double  vec_get(const Vector* v, int i);
void    vec_set(Vector* v, int i, double val);
double  vec_distance(const Vector* a, const Vector* b);
double  vec_angle(const Vector* a, const Vector* b);

#endif
