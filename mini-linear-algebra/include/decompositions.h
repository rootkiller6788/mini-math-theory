#ifndef DECOMPOSITIONS_H
#define DECOMPOSITIONS_H

#include "matrix.h"

void    decompose_qr(const Matrix* A, Matrix** Q, Matrix** R);
void    decompose_qr_mgs(const Matrix* A, Matrix** Q, Matrix** R);
Matrix* decompose_cholesky(const Matrix* A);
Vector* eigen_qr_algorithm(Matrix* A, int max_iter);
Vector* solve_least_squares(const Matrix* A, const Vector* b);

#endif
