#ifndef TENSOR_H
#define TENSOR_H

#include "matrix.h"

typedef struct { int d1, d2, d3; double* data; } Tensor3D;

Tensor3D* tensor3d_create(int d1, int d2, int d3);
void      tensor3d_free(Tensor3D* t);
double    tensor3d_get(const Tensor3D* t, int i, int j, int k);
void      tensor3d_set(Tensor3D* t, int i, int j, int k, double val);
Tensor3D* tensor3d_add(const Tensor3D* a, const Tensor3D* b);
Tensor3D* tensor3d_scale(const Tensor3D* t, double s);
Matrix*   tensor3d_unfold(const Tensor3D* t, int mode);
Tensor3D* tensor3d_fold(const Matrix* m, int d1, int d2, int d3, int mode);
double    tensor3d_frobenius_norm(const Tensor3D* t);
void      tensor3d_print(const Tensor3D* t);

#endif
