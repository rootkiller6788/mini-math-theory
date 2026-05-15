# MIT 18.06 Lecture-by-Lecture Mapping to mini-linear-algebra C Code

A detailed mapping of Prof. Gilbert Strang's MIT 18.06 (Linear Algebra) lectures
to the implementations in this project. Each lecture identifies the corresponding
C source files and key functions that implement the concepts.

---

## Part I: Solving Linear Equations

### Lecture 1: The Geometry of Linear Equations

Row picture and column picture of Ax = b. Linear combinations of columns.

**Our code:** `src/vector.c`, `src/matrix.c`
- `vec_create` — construct a vector
- `vec_scale` — scalar multiplication a * v
- `vec_add` — linear combination v1 + v2
- `mat_create` — construct a matrix
- `mat_multiply` — compute Ax as a linear combination of columns

### Lecture 2: Elimination with Matrices

Elementary row operations expressed as matrix multiplications. Elimination
matrices E_ij. From A to U (upper triangular).

**Our code:** `src/linalg.c`
- `linalg_gaussian_elimination` — forward elimination producing U
- Row operations: subtract multiple of pivot row

### Lecture 3: Multiplication and Inverse Matrices

Four ways to multiply AB: row times column, column times row, columns of A
times rows of B, block multiplication. Inverse of product: (AB)^-1 = B^-1 A^-1.

**Our code:** `src/matrix.c`
- `mat_multiply` — standard dot-product-based matrix multiplication
- `mat_inverse` — compute A^-1 via Gauss-Jordan

### Lecture 4: Factorization into A = LU

LU decomposition: A = LU where L is lower triangular with 1's on diagonal,
U is upper triangular. Cost of elimination: n^3/3 multiplications.

**Our code:** `src/decompositions.c`
- `decomp_lu` — compute L and U factors with partial pivoting (PA = LU)
- `linalg_forward_sub` — solve Ly = b (forward substitution)
- `linalg_back_sub` — solve Ux = y (back substitution)

### Lecture 5: Transposes, Permutations, Spaces R^n

A^T, symmetric matrices A = A^T. Permutation matrices P. P^T = P^-1.
Vector spaces: R^n, subspaces.

**Our code:** `src/matrix.c`
- `mat_transpose` — compute A^T
- `mat_is_symmetric` — check A == A^T
- `vec_*` family — vector operations in R^n

---

## Part II: Vector Spaces and Subspaces

### Lecture 6: Column Space and Nullspace

Column space C(A) = {Ax : x in R^n}. Nullspace N(A) = {x : Ax = 0}.
Subspaces must contain 0 and be closed under addition and scalar multiplication.

**Our code:** `src/matrix.c`
- `mat_column_space` — compute basis for column space
- `mat_null_space` — compute basis for nullspace
- `mat_rank` — dimension of column space

### Lecture 7: Solving Ax = 0 — Pivot Variables, Special Solutions

Complete solution to Ax = 0: special solutions for each free variable.
Rank r means n - r free variables.

**Our code:** `src/matrix.c`, `src/linalg.c`
- `mat_null_space` — returns n - r special solutions
- `linalg_gaussian_elimination` — identifies pivot columns (rank r)

### Lecture 8: Solving Ax = b — Row Reduced Form R

Particular solution plus nullspace solutions: x = x_p + x_n.
Solvability condition: b must be in C(A). Full row rank, full column rank cases.

**Our code:** `src/linalg.c`
- `linalg_solve` — returns particular solution + nullspace basis
- Handles underdetermined, overdetermined, and singular systems

### Lecture 9: Independence, Basis, and Dimension

Linear independence: c1v1 + c2v2 + ... = 0 only if all c_i = 0.
Basis: independent vectors that span the space. Dimension = number of basis vectors.

**Our code:** `src/matrix.c`
- `mat_column_space` — extracts independent columns as basis
- `mat_rank` — returns dimension of column space

### Lecture 10: The Four Fundamental Subspaces

C(A), N(A), C(A^T), N(A^T). Orthogonality: C(A) ⟂ N(A^T), N(A) ⟂ C(A^T).
Dimensions: rank r, n - r, r, m - r.

**Our code:** `src/matrix.c`
- `mat_column_space` — C(A)
- `mat_null_space` — N(A)
- `mat_row_space` — C(A^T)
- `mat_left_null_space` — N(A^T)
- Orthogonality verified via dot products

---

## Part III: Orthogonality

### Lectures 11–12: Orthogonal Vectors and Subspaces

Orthogonal vectors: v^T w = 0. Orthogonal subspaces. Orthogonal complement.

**Our code:** `src/vector.c`
- `vec_dot` — compute v·w; if 0, vectors are orthogonal
- `vec_angle` — angle between vectors

### Lecture 13: Projections onto Lines and Subspaces

Projection onto vector: p = (a^T b / a^T a) a. Projection matrix: P = A(A^T A)^-1 A^T.

**Our code:** `src/matrix.c`
- `mat_project` — compute projection matrix onto column space of A
- `mat_project_vector` — project b onto C(A)

### Lecture 14: Projection Matrices and Least Squares

Least squares: minimize ||Ax - b||^2. Normal equations: A^T A x̂ = A^T b.

**Our code:** `src/linalg.c`
- `linalg_least_squares` — solve normal equations via QR or direct
- Error vector: e = b - A x̂, orthogonal to C(A)

### Lecture 15: Orthogonal Matrices and Gram-Schmidt

Q^T Q = I, columns orthonormal. Gram-Schmidt: from independent a1..an to orthonormal q1..qn.

**Our code:** `src/decompositions.c`
- `decomp_qr_gram_schmidt` — classical Gram-Schmidt
- `decomp_qr_modified_gs` — modified Gram-Schmidt (more stable)
- Both return Q (orthogonal) and R (upper triangular), A = QR

### Lectures 16–17: QR Factorization

A = QR, solving Ax = b via Rx = Q^T b. Applications to least squares.

**Our code:** `src/decompositions.c`
- `decomp_qr_householder` — Householder reflections (most stable)
- `linalg_solve_qr` — solve using QR decomposition

---

## Part IV: Determinants

### Lecture 18: Determinants

Definition, properties: det(I) = 1, sign change on row swap, linear in each row.
Product rule: det(AB) = det(A) det(B).

**Our code:** `src/matrix.c`
- `mat_det` — determinant via LU factorization (det = product of U diagonals × sign of P)

### Lecture 19: Determinant Formulas and Cofactors

Big formula: sum over all n! permutations. Cofactor expansion. Inverse via cofactors.

**Our code:** `src/matrix.c`
- `mat_det` — also available via cofactor expansion for small matrices
- `mat_inverse` — via cofactor as alternative to Gauss-Jordan

### Lecture 20: Cramer's Rule, Volume, and Linear Transformations

Cramer's rule: x_j = det(B_j) / det(A). Volume of parallelepiped = |det(A)|.
Determinant = volume scaling factor of linear transformation.

**Our code:** `src/linalg.c`
- `linalg_solve_cramer` — Cramer's rule solver (educational, not for large n)
- `mat_det` — for computing volumes and scale factors

---

## Part V: Eigenvalues and Eigenvectors

### Lecture 21: Eigenvalues and Eigenvectors

Ax = λx. Characteristic polynomial det(A - λI) = 0. Algebraic multiplicity.

**Our code:** `src/eigen.c`
- `eigen_power_iteration` — find dominant eigenvalue/vector
- `eigen_qr_algorithm` — compute all eigenvalues
- `eigen_characteristic_polynomial` — for small matrices

### Lecture 22: Diagonalization and Powers of A

A = S Λ S^-1. Powers: A^k = S Λ^k S^-1. Markov matrices: λ=1 is largest eigenvalue.

**Our code:** `src/eigen.c`
- `eigen_diagonalize` — compute S and Λ, verify A = SΛS^-1
- Application: `demos/mini-page-rank/` uses power iteration for stationary distribution

### Lecture 23: Differential Equations and e^{At}

du/dt = Au, solution u(t) = e^{At} u(0). e^{At} = S e^{Λt} S^-1.
Stability: Re(λ) < 0.

**Our code:** `src/eigen.c`
- `eigen_matrix_exponential` — compute exp(At) for given t
- Useful for solving linear ODE systems

### Lectures 24–25: Symmetric Matrices and Positive Definite Matrices

Symmetric matrices have real eigenvalues, orthogonal eigenvectors.
A = Q Λ Q^T. Positive definite: x^T A x > 0, all λ > 0.

**Our code:** `src/eigen.c`, `src/decompositions.c`
- `eigen_is_symmetric` — check symmetry
- `decomp_cholesky` — A = L L^T for SPD matrices
- `mat_is_positive_definite` — test via Cholesky attempt or eigenvalues

### Lecture 26: Complex Matrices and Fast Fourier Transform

Complex vectors, Hermitian matrices. Fourier matrix F.

**Our code:** *(future: FFT integration, complex number support)*

### Lectures 27–29: Positive Definite Matrices and Tests

Properties: all λ > 0, all pivots > 0, all upper-left determinants > 0.
Minimum principle: quadratic form ½x^T A x - x^T b minimized at Ax = b.

**Our code:** `src/decompositions.c`
- `decomp_cholesky` — fails if not SPD (practical test)
- `mat_pivots` — all pivots positive iff SPD

---

## Part VI: Singular Value Decomposition

### Lecture 30: Linear Transformations and Their Matrices

Change of basis. Similar matrices A and B = M^-1 A M.

**Our code:** `src/matrix.c`
- `mat_change_basis` — transform vectors between bases
- `mat_is_similar` — check if A = M^-1 B M

### Lectures 31–32: Singular Value Decomposition (SVD)

A = U Σ V^T. U: m×m orthogonal (left singular vectors = eigenvectors of AA^T).
V: n×n orthogonal (right singular vectors = eigenvectors of A^T A).
Σ: m×n diagonal (singular values σ_i = sqrt(λ_i)).

**Our code:** `src/svd.c`
- `svd_decompose` — compute U, Σ, V
- `svd_low_rank_approx` — best rank-k approximation (Eckart-Young)
- `svd_reconstruct` — reconstruct A from rank-k components

### Lectures 33–34: Applications of SVD

Principal Component Analysis. Image compression. Pseudoinverse.
Total least squares.

**Our code / demos:**
- `demos/mini-pca/` — PCA via covariance SVD
- `demos/mini-recommender/` — recommender system via SVD low-rank
- `mat_pseudoinverse` — Moore-Penrose pseudoinverse via SVD

---

## Summary Table: Lectures → Source Files

| Lectures | Topic | Primary Source Files |
|---|---|---|
| 1–2 | Vectors, linear combinations | `vector.c` |
| 3–5 | Matrices, elimination, LU, permutation | `matrix.c`, `linalg.c` |
| 6–10 | Vector spaces, Ax=b, four subspaces | `matrix.c`, `linalg.c` |
| 11–17 | Orthogonality, projections, Gram-Schmidt, QR | `decompositions.c` |
| 18–20 | Determinants | `matrix.c` |
| 21–29 | Eigenvalues, diagonalization, SPD matrices | `eigen.c`, `decompositions.c` |
| 30–34 | Linear transformations, SVD, applications | `matrix.c`, `svd.c` |

All lecture concepts reinforced by `examples/` and `demos/` directories.
