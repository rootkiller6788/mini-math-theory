#ifndef ADVANCED_EIGEN_H
#define ADVANCED_EIGEN_H

#include "matrix.h"
#include <stdbool.h>

/* L5: Arnoldi Iteration — build orthonormal basis V_m of Krylov subspace
 * K_m(A, v1) = span{v1, Av1, ..., A^{m-1}v1} and upper Hessenberg matrix H_m
 * such that A V_m = V_{m+1} H_{m+1,m}. H_m = V_m^T A V_m is the projection.
 * Core of GMRES and Arnoldi eigenvalue methods. Complexity: O(m·nnz + m²n). */
void arnoldi_iteration(const Matrix* A, const Vector* v1, int m,
                       Matrix** V, Matrix** H);

/* L5: Lanczos Iteration — symmetric variant of Arnoldi. Builds tridiagonal T_m
 * such that A V_m = V_m T_m + β_m v_{m+1} e_m^T (three-term recurrence).
 * Requires only O(n) storage per step vs O(mn) for Arnoldi.
 * L4: Lanczos (1950) — minimal polynomial and Krylov. */
void lanczos_iteration(const Matrix* A, const Vector* v1, int m,
                       Vector** alpha, Vector** beta);

/* L5: Implicitly Restarted Arnoldi (IRA) — compute k eigenvalues of largest
 * magnitude by filtering unwanted ones via shifted QR on H_m.
 * L8: Sorensen (1992) — IRA method. Core of ARPACK's eigensolver.
 * This implementation uses exact shifts (unwanted eigenvalues of H_m). */
Vector* arnoldi_eigenvalues(const Matrix* A, int k, int m, int max_iter);

/* L8: Krylov-Schur Restart — stable alternative to explicit restart.
 * Reorders Schur form of H_m to lock converged Ritz values.
 * L8: Stewart (2001) — Krylov-Schur method. */
Vector* krylov_schur_eigenvalues(const Matrix* A, int k, int max_iter);

/* L5: Rayleigh Quotient Iteration (RQI) — cubically convergent for symmetric A.
 * x^{(k+1)} = (A - ρ(x^{(k)}) I)^{-1} x^{(k)} (unnormalized), then normalize.
 * ρ(x) = x^T A x / x^T x (Rayleigh quotient). Convergence: cubic (L4: Ostrowski, 1958). */
double rayleigh_quotient_iteration(const Matrix* A, Vector* x, int max_iter);

/* L5: Symmetric Tridiagonal Eigenvalues — solve using bisection + inverse iteration
 * or QR with Wilkinson shifts. Tridiagonal form enables O(n²) QR steps.
 * L4: Parlett (1980) — The Symmetric Eigenvalue Problem. */
Vector* tridiagonal_eigenvalues(const double* diag, const double* offdiag,
                                 int n, int max_iter);

/* L5: Spectral Radius — compute ρ(A) = max_i |λ_i| via power iteration.
 * Returns estimate of dominant eigenvalue magnitude.
 * L4: Perron-Frobenius theorem — for non-negative matrices, ρ(A) is an eigenvalue. */
double spectral_radius(const Matrix* A, int max_iter);

/* L4: Gershgorin Eigenvalue Bounds — compute λ_min ≤ λ_i ≤ λ_max
 * using Gershgorin disk theorem. Returns 0 on success.
 * L4: Varga (2004) — Geršgorin and His Circles. */
int gershgorin_eigenvalue_bounds(const Matrix* A, double* lambda_min,
                                  double* lambda_max);

/* L5: Shift-and-Invert Spectral Transformation — compute eigenvalues
 * near a given shift σ. Apply power iteration to (A - σI)^{-1}.
 * L8: Used in ARPACK for interior eigenvalue problems. */
double shift_invert_eigenvalue(const Matrix* A, Vector* x,
                                double sigma, int max_iter);

/* L5: Wielandt Deflation — remove converged eigenpair from matrix
 * using a rank-1 update: A - λ v w^T where w is the left eigenvector.
 * More numerically stable than Hotelling deflation (subtraction).
 * L4: Wilkinson (1965) — The Algebraic Eigenvalue Problem. */
void wielandt_deflate(Matrix* A, double lambda, const Vector* v,
                       const Vector* w);

/* L4: Weyl's Inequality (verification) — for Hermitian matrices A, B:
 * λ_i(A) + λ_n(B) ≤ λ_i(A+B) ≤ λ_i(A) + λ_1(B).
 * This function computes bounds and checks consistency. */
int weyl_bounds(const Matrix* A, const Matrix* B, double* lower, double* upper);

#endif
