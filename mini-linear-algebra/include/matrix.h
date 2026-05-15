#ifndef MATRIX_H
#define MATRIX_H

#include "vector.h"

typedef struct { int rows, cols; double* data; } Matrix;

Matrix* mat_create(int rows, int cols);
Matrix* mat_create_identity(int n);
Matrix* mat_create_zeros(int rows, int cols);
Matrix* mat_create_ones(int rows, int cols);
Matrix* mat_from_array(int rows, int cols, const double* data);
void    mat_free(Matrix* m);
Matrix* mat_clone(const Matrix* m);
double  mat_get(const Matrix* m, int r, int c);
void    mat_set(Matrix* m, int r, int c, double val);
Matrix* mat_add(const Matrix* a, const Matrix* b);
Matrix* mat_sub(const Matrix* a, const Matrix* b);
Matrix* mat_scale(const Matrix* m, double s);
Matrix* mat_mul(const Matrix* a, const Matrix* b);
Vector* mat_vec_mul(const Matrix* m, const Vector* v);
Matrix* mat_transpose(const Matrix* m);
double  mat_determinant(Matrix* m);
Matrix* mat_inverse(Matrix* m);
double  mat_trace(const Matrix* m);
double  mat_frobenius_norm(const Matrix* m);
double  mat_condition_number(const Matrix* m);
void    mat_print(const Matrix* m);
Matrix* mat_get_row(const Matrix* m, int r);
Matrix* mat_get_column(const Matrix* m, int c);

#endif
