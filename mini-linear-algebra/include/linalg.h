#ifndef LINALG_H
#define LINALG_H

#include "matrix.h"
#include <stdbool.h>

Vector* solve_ax_b(Matrix* A, Vector* b);
bool    decompose_lu(Matrix* A, int* pivot);
Vector* solve_lu(const Matrix* LU, const int* pivot, const Vector* b);
double  mat_rank(Matrix* m);
Vector* mat_nullspace_vector(Matrix* m);
void    mat_col_space(Matrix* m);
void    mat_row_space(Matrix* m);

#endif
