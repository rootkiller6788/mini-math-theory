#ifndef EIGEN_H
#define EIGEN_H

#include "matrix.h"

double  eigen_power_iteration(const Matrix* A, Vector* eigenvector, int max_iter);
double  eigen_inverse_power(const Matrix* A, Vector* eigenvector, double shift, int max_iter);
double  eigen_rayleigh_quotient(const Matrix* A, const Vector* x);
Vector* eigen_all(const Matrix* A, int max_iter);
void    eigen_deflate(Matrix* A, double eigenvalue, const Vector* eigenvector);

#endif
