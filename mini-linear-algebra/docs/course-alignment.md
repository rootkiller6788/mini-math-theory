# Course Alignment

How `mini-linear-algebra` modules map to university linear algebra courses.

## MIT 18.06 — Linear Algebra (Prof. Gilbert Strang)

### Part 1: Vector Spaces and Solving Ax = b

| 18.06 Topic | Our Module | Key Operations |
|---|---|---|
| Vectors, dot product, norms | `vector.h` | `vec_dot`, `vec_norm`, `vec_angle` |
| Matrix multiplication, transpose | `matrix.h` | `mat_multiply`, `mat_transpose` |
| Gaussian elimination, echelon form | `linalg.h` | `linalg_gaussian_elimination` |
| Solving Ax = b | `linalg.h` | `linalg_solve`, `linalg_lu` |
| LU decomposition | `decompositions.h` | `decomp_lu` |
| Vector spaces, subspaces | `matrix.h` | `mat_column_space`, `mat_null_space` |
| Rank, dimension, nullity | `matrix.h` | `mat_rank` |

### Part 2: Orthogonality and Determinants

| 18.06 Topic | Our Module | Key Operations |
|---|---|---|
| Orthogonal vectors, projections | `vector.h` | `vec_dot`, `vec_angle`, `vec_normalize` |
| QR decomposition | `decompositions.h` | `decomp_qr_gram_schmidt`, `decomp_qr_modified_gs`, `decomp_qr_householder` |
| Least squares | `linalg.h` | Solve normal equations via QR |
| Determinants, properties | `matrix.h` | `mat_det` |
| Cramer's rule, cofactors | `matrix.h` | `mat_inverse` via cofactor expansion |

### Part 3: Eigenvalues and SVD

| 18.06 Topic | Our Module | Key Operations |
|---|---|---|
| Eigenvalues, eigenvectors | `eigen.h` | `eigen_power_iteration`, `eigen_qr_algorithm`, `eigen_rayleigh_quotient` |
| Diagonalization | `eigen.h` | Build diagonal matrix from eigenvalues |
| Symmetric matrices | `eigen.h` | Real eigenvalues, orthogonal eigenvectors |
| Positive definite matrices | `decompositions.h` | `decomp_cholesky` |
| Singular Value Decomposition | `svd.h` | `svd_decompose`, `svd_low_rank_approx` |
| Matrix norms, condition number | `matrix.h` | `mat_norm`, `mat_condition_number` |

### Part 4: Applications

| 18.06 Topic | Our Example / Demo |
|---|---|
| Linear transformations (rotation, scaling) | `demos/mini-graphics-transform/` |
| Graphs and networks | `demos/mini-page-rank/` |
| Linear programming | Can be built on top of linalg primitives |
| Differential equations | Matrix exponential via eigenvalues |
| Markov matrices | `demos/mini-page-rank/` (stochastic matrix) |

---

## MIT 18.065 — Matrix Methods in Data Analysis, Signal Processing, and ML

| 18.065 Topic | Our Module | Key Operations |
|---|---|---|
| Column spaces, four fundamental subspaces | `matrix.h` | `mat_column_space`, `mat_null_space`, `mat_row_space`, `mat_left_null_space` |
| Least squares and weighted least squares | `linalg.h`, `decompositions.h` | QR + normal equations |
| Eigenvalues of symmetric matrices | `eigen.h` | Spectral theorem in practice |
| Positive definite matrices | `decompositions.h` | `decomp_cholesky` |
| SVD — principal component analysis | `svd.h` | `svd_decompose`, `svd_low_rank_approx` |
| Low-rank matrix completion (Netflix problem) | `demos/mini-recommender/` | SVD for collaborative filtering |
| Norms and condition numbers | `matrix.h` | `mat_norm`, `mat_condition_number` |
| Randomized linear algebra | `svd.h` | Randomized SVD (future) |
| Tensors and multi-dimensional data | `tensor.h` | `tensor_contract` |
| Sparse matrices, compressed sensing | `sparse.h` | CSR format, sparse matvec |

---

## Stanford CS229 — Machine Learning (Linear Algebra Review)

| CS229 Topic | Our Module / Demo | Notes |
|---|---|---|
| Vectors, inner products, norms | `vector.h` | `vec_dot`, `vec_norm`, `vec_norm_p`, `vec_distance` |
| Matrices, matrix multiplication | `matrix.h` | `mat_multiply` — fundamental to neural nets |
| Matrix inverse, pseudoinverse | `linalg.h`, `svd.h` | Normal equations for least squares |
| Trace and determinant | `matrix.h` | `mat_trace`, `mat_det` |
| Eigenvalues, eigenvectors | `eigen.h` | `eigen_power_iteration`, `eigen_qr_algorithm` |
| Symmetric matrices, spectral theorem | `eigen.h` | All eigenvalues real, orthogonal eigenvectors |
| Positive (semi-)definite matrices | `decompositions.h` | `decomp_cholesky` as definiteness test |
| SVD, PCA | `svd.h`, `demos/mini-pca/` | Dimensionality reduction |
| Covariance matrix | `demos/mini-pca/` | `X^T X / (n-1)` |
| Gradient descent uses linear algebra | `demos/mini-neural-layer/` | Wx + b as core operation |
| Least squares regression | `examples/` | Normal equations: theta = (X^T X)^-1 X^T y |

---

## Summary Table

| Concept | `mini-linear-algebra` Module |
|---|---|
| Vector operations | `vector.h` |
| Matrix operations, rank, subspaces | `matrix.h` |
| Solving linear systems, elimination | `linalg.h` |
| LU, QR, Cholesky decompositions | `decompositions.h` |
| Eigenvalues, eigenvectors | `eigen.h` |
| Singular Value Decomposition | `svd.h` |
| Tensors, contractions | `tensor.h` |
| Sparse matrix operations | `sparse.h` |
