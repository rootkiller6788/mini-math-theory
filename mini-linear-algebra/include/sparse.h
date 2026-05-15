#ifndef SPARSE_H
#define SPARSE_H

#include "matrix.h"

typedef struct {
    int* row_ptr;
    int* col_ind;
    double* values;
    int rows, cols, nnz;
} SparseMatrix;

SparseMatrix* sparse_create(int rows, int cols, int nnz_max);
void          sparse_free(SparseMatrix* s);
void          sparse_add_entry(SparseMatrix* s, int r, int c, double val);
void          sparse_finalize(SparseMatrix* s);
double        sparse_get(const SparseMatrix* s, int r, int c);
Vector*       sparse_mul_vec(const SparseMatrix* s, const Vector* v);
Matrix*       sparse_to_dense(const SparseMatrix* s);
SparseMatrix* dense_to_sparse(const Matrix* m);
void          sparse_print(const SparseMatrix* s);

#endif
