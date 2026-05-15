# Numerical Stability in mini-linear-algebra

A practical guide to the numerical considerations implemented in this library
and the reasoning behind each design choice.

---

## Pivoting in Gaussian Elimination

### The Problem

Without pivoting, elimination can fail catastrophically:

```
System: [ ε  1 ] [x1] = [ 1 ]
         [ 1  1 ] [x2]   [ 2 ]

Without pivoting: multiply row 1 by 1/ε, subtract from row 2.
For small ε (e.g., 10^-16), 1/ε is huge, amplifying roundoff error.

U = [ ε        1     ]
    [ 0    1 - 1/ε   ]  ≈  [ ε    1  ]
                           [ 0   -1/ε ]

x2 ≈ 1, x1 ≈ 0  — catastrophically wrong.
```

### The Solution: Partial Pivoting

At each elimination step k, find row p (p ≥ k) such that |A[p][k]| is maximized.
Swap rows k and p. This ensures:

1. **No division by zero** — unless the entire column is zero (singular matrix)
2. **Multiplier ≤ 1** — since |A[i][k]| / |A[k][k]| ≤ 1 after pivot selection
3. **Error growth is controlled** — roundoff error at step k is bounded by the
   growth factor ρ = max_{i,j} |U[i][j]| / max_{i,j} |A[i][j]|

**Our implementation:** `src/linalg.c` — `linalg_gaussian_elimination` uses
partial pivoting. The pivot index is recorded in a permutation array for
later use in forward/back substitution.

```c
// Partial pivoting example
int pivot = k;
double max_val = fabs(A[k][k]);
for (int i = k + 1; i < n; i++) {
    if (fabs(A[i][k]) > max_val) {
        max_val = fabs(A[i][k]);
        pivot = i;
    }
}
if (pivot != k) mat_swap_rows(A, k, pivot);
```

### Complete Pivoting vs Partial

- **Partial pivoting** (O(n^2)): swap rows only, nearly always sufficient
- **Complete pivoting** (O(n^3)): swap both rows and columns, rarely needed
- This library uses partial pivoting for performance and sufficient stability

---

## Gram-Schmidt: Classical vs Modified

Both methods compute the QR decomposition A = QR, but they differ in numerical
behavior for ill-conditioned matrices.

### Classical Gram-Schmidt (CGS)

```
for j = 1 to n:
    v_j = a_j
    for i = 1 to j-1:
        r_ij = q_i^T a_j
        v_j = v_j - r_ij q_i
    r_jj = ||v_j||
    q_j = v_j / r_jj
```

**Problem:** All projections onto previous q_i happen simultaneously. When A
is nearly rank-deficient, the computed Q loses orthogonality. For the
10th-order Hilbert matrix (condition number ~10^13), ||Q^T Q - I|| can
exceed 10^-3 with CGS.

### Modified Gram-Schmidt (MGS)

```
for j = 1 to n:
    v_j = a_j
for i = 1 to n:
    r_ii = ||v_i||
    q_i = v_i / r_ii
    for j = i+1 to n:
        r_ij = q_i^T v_j
        v_j = v_j - r_ij q_i
```

**Improvement:** Each q_i is subtracted from all remaining v_j immediately
after being computed. This reduces accumulated roundoff error. For the same
Hilbert matrix, MGS gives ||Q^T Q - I|| ~ 10^-6, roughly √ε_mach improvement.

### Householder Reflections (Best Stability)

```
H = I - 2 v v^T    (v is a unit vector)
H^T = H = H^-1     (symmetric, orthogonal, involutory)

Apply sequence H_n ... H_2 H_1 A = R (upper triangular)
Then Q = H_1 H_2 ... H_n
```

Householder provides guaranteed orthogonality: ||Q^T Q - I|| ≈ ε_mach (machine
epsilon), independent of condition number. The cost is higher: 2n^3/3 for
Householder vs n^3 for MGS, but the stability gain is worth it for
ill-conditioned matrices.

**Our implementations:** `src/decompositions.c`
- `decomp_qr_gram_schmidt` — classical, for well-conditioned matrices
- `decomp_qr_modified_gs` — modified, default QR choice
- `decomp_qr_householder` — Householder, for critical applications

---

## Condition Number

### Definition

```
κ(A) = ||A|| · ||A^-1||
```

For the 2-norm (spectral norm):
```
κ_2(A) = σ_max / σ_min
```

where σ_max, σ_min are the largest and smallest singular values of A.

### Interpretation

- κ = 1: perfectly conditioned (identity matrix, orthogonal matrix)
- κ small (< 10^3): well-conditioned, reliable solution
- κ large (> 10^8): ill-conditioned, solution may have few correct digits
- Roundoff amplifies relative error by factor of κ: Δx/x ≤ κ · Δb/b

### Rule of Thumb

If κ(A) ≈ 10^k, you lose approximately k decimal digits of accuracy when
solving Ax = b in double precision (≈16 digits).

**Our implementation:** `src/matrix.c`, `src/svd.c`

```c
double cond = mat_condition_number(A);  // via SVD: σ_max / σ_min
if (cond > 1e12) {
    fprintf(stderr, "Warning: ill-conditioned matrix, κ = %.2e\n", cond);
}
```

---

## Cholesky vs LU Decomposition

### Cholesky: A = L L^T (Lower Triangular × Its Transpose)

**When to use:**
- Matrix **must** be symmetric positive definite (SPD)
- 2× faster than LU (n^3/3 vs 2n^3/3 operations)
- Requires only lower triangular storage (n(n+1)/2 entries)
- No pivoting needed (SPD matrices have positive pivots)
- Stable without pivoting: error bound ||A - L L^T|| ≤ c(n) ε ||A||

**When NOT to use:**
- Non-symmetric matrices
- Indefinite matrices (mixed sign eigenvalues)
- Near-singular SPD matrices (Cholesky will detect but is unstable)

**Our implementation:** `src/decompositions.c`

```c
CholeskyResult* L = decomp_cholesky(A);
if (L == NULL) {
    // Not SPD or singular — fall back to LU
    LUResult* lu = decomp_lu(A);
}
```

### LU: A = P L U (with Pivoting)

**When to use:**
- Most general method
- Any square nonsingular matrix
- When symmetry/definiteness is unknown or fails

**Tradeoff:**
- Slower (2n^3/3)
- Requires pivoting overhead: permutation tracking, row swaps
- Can handle non-symmetric, indefinite systems

### Decision Table

| Matrix Type | Recommended Decomposition |
|---|---|
| General square, nonsingular | LU with partial pivoting |
| Symmetric positive definite | Cholesky |
| Symmetric indefinite | LDL^T (Bunch-Kaufman) — future |
| Overdetermined least squares | QR (Householder) |
| Underdetermined, rank-deficient | SVD pseudoinverse |

---

## Power Iteration Convergence

### The Algorithm

```
v^(0) = random vector
for k = 0, 1, 2, ...:
    w = A v^(k)
    v^(k+1) = w / ||w||
    λ^(k+1) = (v^(k+1))^T A v^(k+1)
```

### Convergence Rate

The error decreases geometrically with ratio:

```
|λ_2 / λ_1|
```

where |λ_1| > |λ_2| ≥ ... ≥ |λ_n|.

**Example:** If |λ_2 / λ_1| = 0.1, each iteration gains ~1 decimal digit.
After 10 iterations: ~10 digits accuracy.

### Convergence Isues

| Issue | Mitigation |
|---|---|
| |λ_2| ≈ |λ_1| | Slow convergence; use inverse iteration or QR algorithm |
| λ_1 complex | Power iteration fails; use QR algorithm for complex eigenvalues |
| v_0 ⟂ x_1 | Eventually, roundoff introduces component in direction of x_1 |
| Multiple dominant eigenvalues | Cyclic behavior; subspace iteration or QR algorithm |
| Symmetric? | Rayleigh quotient gives quadratic convergence near solution |

**Our implementation:** `src/eigen.c`

```c
// Track convergence via eigenvalue change
double old_lambda = lambda;
lambda = eigen_rayleigh_quotient(A, v);
if (fabs(lambda - old_lambda) < tol) break; // converged
```

### Alternative Methods in This Library

| Method | Best For |
|---|---|
| `eigen_power_iteration` | Largest eigenvalue (in magnitude) |
| `eigen_inverse_iteration` | Eigenvalue closest to shift μ |
| `eigen_rayleigh_quotient_iteration` | Cubic convergence for symmetric matrices |
| `eigen_qr_algorithm` | All eigenvalues simultaneously |
| `eigen_subspace_iteration` | Several largest eigenvalues |

---

## Summary of Stability Recommendations

| Operation | Default Choice | Fallback |
|---|---|---|
| Solve Ax = b | LU with partial pivoting | SVD pseudoinverse (if rank-deficient) |
| QR decomposition | Modified Gram-Schmidt | Householder (if orthogonality critical) |
| Eigenvalues (dominant) | Power iteration + deflation | QR algorithm (if all needed) |
| Cholesky | Direct (if SPD confirmed) | LU (if SPD fails) |
| Least squares | QR (Householder or MGS) | SVD (if rank-deficient design matrix) |
| Matrix inverse | LU → solve AX = I | SVD pseudoinverse |

### Machine Epsilon Reference

| Precision | ε_mach | ≈ Digits |
|---|---|---|
| float (32-bit) | ~1.19e-7 | 6-7 |
| double (64-bit) | ~2.22e-16 | 15-16 |
| long double (80-bit x87) | ~1.08e-19 | 18-19 |

This library uses `double` throughout.
