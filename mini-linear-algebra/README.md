# mini-linear-algebra

MIT 18.06 + 18.065 + CS229 linear algebra implemented in C.

A self-contained, zero-dependency C library that covers the core topics of
undergraduate and graduate linear algebra, with emphasis on numerical stability
and practical applications in machine learning and scientific computing.

## Modules

| Module | Header | Description |
|---|---|---|
| **Vector** | `vector.h` | Vector creation, arithmetic, dot/cross product, norms, distances |
| **Matrix** | `matrix.h` | Matrix creation, multiplication, transpose, rank, null space, column space |
| **Linear Algebra** | `linalg.h` | Gaussian elimination, LU decomposition, solving Ax=b, matrix inverse |
| **Decompositions** | `decompositions.h` | LU, QR (Gram-Schmidt, Householder), Cholesky |
| **Eigen** | `eigen.h` | Eigenvalues and eigenvectors via power iteration, QR algorithm |
| **SVD** | `svd.h` | Singular value decomposition, low-rank approximation |
| **Tensor** | `tensor.h` | Multi-dimensional array operations, tensor contractions |
| **Sparse** | `sparse.h` | Compressed sparse row (CSR) format, sparse matrix-vector multiplication |

## Quick Reference

| Module | Key Functions |
|---|---|
| `vector.c` | `vec_create`, `vec_dot`, `vec_norm`, `vec_normalize`, `vec_cross3`, `vec_add`, `vec_sub`, `vec_scale`, `vec_angle`, `vec_distance` |
| `matrix.c` | `mat_create`, `mat_multiply`, `mat_transpose`, `mat_rank`, `mat_inverse`, `mat_det`, `mat_trace`, `mat_print` |
| `linalg.c` | `linalg_solve`, `linalg_gaussian_elimination`, `linalg_lu`, `linalg_forward_sub`, `linalg_back_sub` |
| `decompositions.c` | `decomp_lu`, `decomp_qr_gram_schmidt`, `decomp_qr_modified_gs`, `decomp_qr_householder`, `decomp_cholesky` |
| `eigen.c` | `eigen_power_iteration`, `eigen_inverse_iteration`, `eigen_qr_algorithm`, `eigen_rayleigh_quotient` |
| `svd.c` | `svd_decompose`, `svd_low_rank_approx`, `svd_reconstruct` |
| `tensor.c` | `tensor_create`, `tensor_get`, `tensor_set`, `tensor_contract`, `tensor_matmul` |
| `sparse.c` | `sparse_create_csr`, `sparse_matvec`, `sparse_free` |

## Directory Structure

```
mini-linear-algebra/
├── include/            Header files (.h)
│   ├── vector.h
│   ├── matrix.h
│   ├── linalg.h
│   ├── decompositions.h
│   ├── eigen.h
│   ├── svd.h
│   ├── tensor.h
│   └── sparse.h
├── src/                Implementation files (.c)
│   ├── vector.c
│   ├── matrix.c
│   ├── linalg.c
│   ├── decompositions.c
│   ├── eigen.c
│   ├── svd.c
│   ├── tensor.c
│   └── sparse.c
├── examples/           Standalone example programs
│   ├── solve_axb.c
│   ├── pca_demo.c
│   ├── pagerank.c
│   ├── svd_recommender.c
│   ├── graphics_transform.c
│   └── neural_layer.c
├── demos/              Demo READMEs and example data
│   ├── mini-pca/
│   ├── mini-recommender/
│   ├── mini-graphics-transform/
│   ├── mini-neural-layer/
│   └── mini-page-rank/
├── tests/              Unit tests
├── benches/            Performance benchmarks
├── docs/               Documentation
│   ├── course-alignment.md
│   ├── mit-18.06-map.md
│   ├── cs229-linear-algebra-map.md
│   ├── numerical-stability.md
│   └── applications.md
├── Makefile
└── README.md
```

## Building

### Prerequisites

- C compiler (GCC, Clang, or MSVC)
- GNU Make or compatible

### Build All Examples

```
make all
```

Compiles the static library `liblinalg.a` and all examples in `examples/`.

### Run Tests

```
make test
```

Builds and runs all example programs, verifying output against expected results.

### Build a Specific Example

```
make examples/solve_axb
```

### Clean

```
make clean
```

Removes all object files, the static library, and compiled examples.

## Numerical Approach

This library prioritizes numerical stability:

- **Partial pivoting** in Gaussian elimination prevents division by zero and
  limits growth of roundoff errors.
- **Modified Gram-Schmidt** for QR decomposition reduces accumulated error
  compared to classical Gram-Schmidt.
- **Householder reflections** are provided as an alternative QR method with
  even better stability.
- Condition number estimation is available to assess problem sensitivity.
- The Cholesky decomposition is used for symmetric positive definite matrices
  where it is numerically superior to LU.

## Course Alignment

See [`docs/course-alignment.md`](docs/course-alignment.md) for a detailed
mapping of modules to MIT 18.06, MIT 18.065, and Stanford CS229.
