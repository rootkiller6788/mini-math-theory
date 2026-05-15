#include "sparse.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPS 1e-10

typedef struct {
    int r, c;
    double val;
} Triplet;

static int cmp_triplet(const void* a, const void* b) {
    const Triplet* ta = (const Triplet*)a;
    const Triplet* tb = (const Triplet*)b;
    if (ta->r != tb->r) return ta->r - tb->r;
    return ta->c - tb->c;
}

SparseMatrix* sparse_create(int rows, int cols, int nnz_max) {
    SparseMatrix* s = (SparseMatrix*)malloc(sizeof(SparseMatrix));
    s->rows = rows;
    s->cols = cols;
    s->nnz = 0;
    s->row_ptr = (int*)calloc(rows + 1, sizeof(int));
    s->col_ind = (int*)malloc(nnz_max * sizeof(int));
    s->values = (double*)malloc(nnz_max * sizeof(double));
    return s;
}

void sparse_free(SparseMatrix* s) {
    if (s) {
        free(s->row_ptr);
        free(s->col_ind);
        free(s->values);
        free(s);
    }
}

void sparse_add_entry(SparseMatrix* s, int r, int c, double val) {
    if (fabs(val) < EPS) return;
    s->col_ind[s->nnz] = c;
    s->values[s->nnz] = val;
    s->row_ptr[r + 1]++;
    s->nnz++;
}

void sparse_finalize(SparseMatrix* s) {
    for (int i = 0; i < s->rows; i++)
        s->row_ptr[i + 1] += s->row_ptr[i];

    Triplet* trips = (Triplet*)malloc(s->nnz * sizeof(Triplet));
    int* cur_ptr = (int*)malloc(s->rows * sizeof(int));
    memcpy(cur_ptr, s->row_ptr, s->rows * sizeof(int));

    int idx = 0;
    for (int r = 0; r < s->rows; r++) {
        for (int k = s->row_ptr[r]; k < s->row_ptr[r + 1]; k++) {
            trips[idx].r = r;
            trips[idx].c = s->col_ind[idx];
            trips[idx].val = s->values[idx];
            idx++;
        }
    }

    qsort(trips, s->nnz, sizeof(Triplet), cmp_triplet);

    for (int i = 0; i < s->nnz; i++) {
        s->col_ind[i] = trips[i].c;
        s->values[i] = trips[i].val;
    }

    free(trips);
    free(cur_ptr);
}

double sparse_get(const SparseMatrix* s, int r, int c) {
    for (int j = s->row_ptr[r]; j < s->row_ptr[r + 1]; j++)
        if (s->col_ind[j] == c)
            return s->values[j];
    return 0.0;
}

Vector* sparse_mul_vec(const SparseMatrix* s, const Vector* v) {
    Vector* r = vec_create(s->rows);
    for (int i = 0; i < s->rows; i++) {
        double sum = 0.0;
        for (int j = s->row_ptr[i]; j < s->row_ptr[i + 1]; j++)
            sum += s->values[j] * v->data[s->col_ind[j]];
        r->data[i] = sum;
    }
    return r;
}

Matrix* sparse_to_dense(const SparseMatrix* s) {
    Matrix* m = mat_create(s->rows, s->cols);
    for (int i = 0; i < s->rows; i++)
        for (int j = s->row_ptr[i]; j < s->row_ptr[i + 1]; j++)
            m->data[i * s->cols + s->col_ind[j]] = s->values[j];
    return m;
}

SparseMatrix* dense_to_sparse(const Matrix* m) {
    int nnz = 0;
    for (int i = 0; i < m->rows * m->cols; i++)
        if (fabs(m->data[i]) > EPS)
            nnz++;

    SparseMatrix* s = sparse_create(m->rows, m->cols, nnz);
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            double val = m->data[i * m->cols + j];
            if (fabs(val) > EPS) {
                sparse_add_entry(s, i, j, val);
            }
        }
    }
    sparse_finalize(s);
    return s;
}

void sparse_print(const SparseMatrix* s) {
    printf("SparseMatrix %dx%d, nnz=%d\n", s->rows, s->cols, s->nnz);
    printf("Row pointers: [");
    for (int i = 0; i <= s->rows; i++) {
        printf("%d", s->row_ptr[i]);
        if (i < s->rows) printf(" ");
    }
    printf(" ]\n");
    printf("Col indices:  [");
    for (int i = 0; i < s->nnz; i++) {
        printf("%d", s->col_ind[i]);
        if (i < s->nnz - 1) printf(" ");
    }
    printf(" ]\n");
    printf("Values:       [");
    for (int i = 0; i < s->nnz; i++) {
        printf("%.4f", s->values[i]);
        if (i < s->nnz - 1) printf(" ");
    }
    printf(" ]\n");
}
