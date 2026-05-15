# Stanford CS229 Linear Algebra Review → mini-linear-algebra C Mapping

This document maps each topic from the CS229 (Machine Learning) linear algebra
review / refresher to the corresponding C implementation in this project.

CS229 assumes familiarity with: vectors, matrices, matrix-vector multiplication,
eigenvalues/eigenvectors, matrix decompositions, and basic operations used
throughout machine learning.

---

## 1. Vectors and Vector Operations

**CS229 concepts:**
- Vectors in R^n, notation x = [x_1, x_2, ..., x_n]^T
- Inner (dot) product: x^T y = Σ x_i y_i
- Euclidean norm: ||x||_2 = sqrt(x^T x)
- L_p norms: ||x||_p = (Σ |x_i|^p)^(1/p)
- Vector addition: x + y
- Scalar multiplication: αx
- Orthogonal vectors: x^T y = 0
- Cauchy-Schwarz: |x^T y| ≤ ||x|| · ||y||
- Angle between vectors: cos θ = (x^T y) / (||x|| · ||y||)

**Our code:** `src/vector.c`
```c
Vector* v = vec_from_array(data, n);
double dot = vec_dot(a, b);
double norm = vec_norm(v);
double norm_p = vec_norm_p(v, 2.0);
Vector* sum = vec_add(a, b);
Vector* scaled = vec_scale(v, alpha);
double angle = vec_angle(a, b);
double dist = vec_distance(a, b);
```

---

## 2. Matrices and Matrix Operations

**CS229 concepts:**
- Matrix A ∈ R^(m×n), entry A_ij
- Matrix-vector product: (Ax)_i = Σ_j A_ij x_j
- Matrix-matrix product: (AB)_ik = Σ_j A_ij B_jk
- Associativity: (AB)C = A(BC)
- Distributivity: A(B+C) = AB + AC
- NOT commutative: AB ≠ BA in general
- Transpose: (A^T)_ij = A_ji
- Symmetric matrix: A^T = A
- Identity matrix I, I A = A I = A
- Matrix inverse: A A^-1 = A^-1 A = I
- (A^T)^-1 = (A^-1)^T

**Our code:** `src/matrix.c`
```c
Matrix* A = mat_create(m, n);
Matrix* C = mat_multiply(A, B);
Vector* y = mat_vec_multiply(A, x);
Matrix* AT = mat_transpose(A);
int sym = mat_is_symmetric(A);
Matrix* inv = mat_inverse(A);
```

---

## 3. Trace and Determinant

**CS229 concepts:**
- Trace: tr(A) = Σ_i A_ii
- Trace properties: tr(A+B) = tr(A) + tr(B), tr(cA) = c·tr(A)
- Cyclic property: tr(ABC) = tr(BCA) = tr(CAB)
- Determinant: det(A) for square matrices
- det(A^T) = det(A)
- det(AB) = det(A) det(B)
- det(A^-1) = 1 / det(A)
- For 2×2 matrix: det = ad - bc

**Our code:** `src/matrix.c`
```c
double trace = mat_trace(A);
double det = mat_det(A);
```

---

## 4. Eigenvalues and Eigenvectors

**CS229 concepts:**
- Ax = λx, where λ is eigenvalue, x is eigenvector
- Characteristic equation: det(A - λI) = 0
- Eigenvectors for distinct eigenvalues are linearly independent
- Spectral theorem: for symmetric A, all λ are real, eigenvectors orthogonal
- Eigendecomposition: A = Q Λ Q^T (symmetric case)
- Positive (semi-)definite: all λ > 0 (≥ 0)
- x^T A x = Σ λ_i (q_i^T x)^2

**Our code:** `src/eigen.c`
```c
EigenResult* res = eigen_qr_algorithm(A, n);  // all eigenvalues
EigenPair* pair = eigen_power_iteration(A, n); // dominant eigenvalue/vector
double rq = eigen_rayleigh_quotient(A, v);      // Rayleigh quotient
```

---

## 5. Matrix Calculus (Gradient Context)

**CS229 concepts:**
- Gradient of quadratic form: ∇_x (x^T A x) = (A + A^T) x
- For symmetric A: ∇_x (x^T A x) = 2 A x
- Gradient of linear form: ∇_x (b^T x) = b
- Least squares gradient: ∇_θ ||Xθ - y||^2 = 2 X^T (Xθ - y)

**Our code:** `src/matrix.c`, `src/linalg.c`
```c
// Normal equations solve (gradient = 0)
Matrix* XTX = mat_multiply(mat_transpose(X), X);
Vector* XTy = mat_vec_multiply(mat_transpose(X), y);
Vector* theta = linalg_solve(XTX, XTy);
```

---

## 6. Singular Value Decomposition (SVD)

**CS229 concepts:**
- A = U Σ V^T
- U ∈ R^(m×m), Σ ∈ R^(m×n), V ∈ R^(n×n)
- U^T U = I, V^T V = I (orthogonal)
- Singular values σ_i = Σ_ii, σ_1 ≥ σ_2 ≥ ... ≥ 0
- Rank of A = number of nonzero singular values
- Best rank-k approximation: A_k = Σ_{i=1}^k σ_i u_i v_i^T
- Frobenius norm: ||A||_F = sqrt(Σ σ_i^2)
- Condition number: κ(A) = σ_max / σ_min

**Our code:** `src/svd.c`
```c
SVDResult* svd = svd_decompose(A);
Matrix* A_k = svd_low_rank_approx(svd, k); // rank-k best approx
double cond = svd->sigma[0] / svd->sigma[r-1]; // condition number
```

---

## 7. Principal Component Analysis (PCA)

**CS229 concepts:**
1. Center data: subtract mean from each feature
2. Compute covariance matrix: Σ = (1/n) X^T X (assuming zero-mean)
3. Eigendecomposition of Σ: Σ = V Λ V^T
4. Top k eigenvectors = principal components
5. Project data: X_proj = X V_k

**Our code:** `demos/mini-pca/`, `src/eigen.c`, `src/svd.c`
```c
// Option A: via covariance eigendecomposition
Matrix* centered = mat_subtract_mean(X);
Matrix* cov = mat_covariance(centered);
EigenResult* eig = eigen_qr_algorithm(cov, d);
Matrix* proj = mat_multiply(X, eig->vectors_top_k);

// Option B: via SVD of centered data (more stable)
SVDResult* svd = svd_decompose(centered);
// V columns = principal components, singular values squared = eigenvalues of cov
```

---

## 8. Positive Definite Matrices

**CS229 concepts:**
- A ≻ 0 means x^T A x > 0 for all x ≠ 0
- Equivalent: all eigenvalues > 0
- Equivalent: Cholesky decomposition exists (A = L L^T)
- Common in ML: Hessian is SPD at local minimum
- Gaussian covariance matrix must be SPD

**Our code:** `src/decompositions.c`, `src/eigen.c`
```c
CholeskyResult* chol = decomp_cholesky(A); // succeeds iff A is SPD
int is_pd = (chol != NULL);
// or check eigenvalues
EigenResult* eig = eigen_qr_algorithm(A, n);
int is_pd = (eig->min_eigenvalue > 0);
```

---

## 9. Least Squares

**CS229 concepts:**
- Given design matrix X (n×d) and targets y (n×1)
- Minimize J(θ) = ||Xθ - y||_2^2
- Normal equations: X^T X θ = X^T y
- Solution: θ = (X^T X)^-1 X^T y
- Regularized: θ = (X^T X + λI)^-1 X^T y (ridge regression)

**Our code:** `src/linalg.c`, `examples/`
```c
// Normal equations approach
Matrix* XTX = mat_multiply(X_T, X);
Vector* XTy = mat_vec_multiply(X_T, y);
Vector* theta = linalg_solve(XTX, XTy);

// QR approach (more stable)
QRResult* qr = decomp_qr_householder(X);
Vector* theta = linalg_solve_qr(qr, y);
```

---

## 10. Neural Network Layer (Linear Algebra Foundation)

**CS229 concepts:**
- A fully-connected layer is: z = W x + b
- W ∈ R^(d_out × d_in): weight matrix
- x ∈ R^d_in: input
- b ∈ R^d_out: bias
- Activation: a = σ(z), where σ is element-wise (ReLU, sigmoid, tanh)

**Our code:** `demos/mini-neural-layer/`
```c
Vector* z = mat_vec_multiply(W, x);  // z = Wx
vec_add_inplace(z, b);               // z = Wx + b
vec_apply_relu(z);                   // a = ReLU(z), element-wise
```

---

## Summary: CS229 Concept → Source File

| CS229 Topic | Source File |
|---|---|
| Vectors, inner product, norms | `src/vector.c` |
| Matrices, multiplication, transpose, inverse | `src/matrix.c` |
| Trace, determinant | `src/matrix.c` |
| Eigendecomposition, eigenvalues | `src/eigen.c` |
| Positive definite matrices, Cholesky | `src/decompositions.c` |
| SVD | `src/svd.c` |
| PCA | `demos/mini-pca/`, `src/svd.c` |
| Least squares, normal equations | `src/linalg.c` |
| QR decomposition | `src/decompositions.c` |
| Neural network layer (Wx + b) | `demos/mini-neural-layer/` |
| Covariance matrix | `src/matrix.c` |
| Pseudoinverse | `src/svd.c` |
| Matrix calculus (gradients) | `src/matrix.c`, `src/linalg.c` |
