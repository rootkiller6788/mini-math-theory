#ifndef SVD_H
#define SVD_H

#include "matrix.h"

void    svd_power_method(const Matrix* A, Matrix** U, Vector** Sigma, Matrix** Vt,
                         int k, int max_iter);
Matrix* svd_low_rank_approx(const Matrix* A, int k);
Matrix* svd_reconstruct(const Matrix* U, const Vector* Sigma, const Matrix* Vt, int k);

#endif
