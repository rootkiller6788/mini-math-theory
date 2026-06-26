#ifndef ITERATIVE_H
#define ITERATIVE_H

#include "matrix.h"
#include <stdbool.h>

/* L5: Jacobi Iteration — solves Ax = b by splitting A = D + L + U.
 * Each component updated independently: x_i^{(k+1)} = (b_i - sum_{j≠i} a_{ij} x_j^{(k)}) / a_{ii}.
 * Convergence guaranteed for strictly diagonally dominant matrices (L4: Jacobi Convergence Theorem).
 * Complexity: O(n²) per iteration. */
Vector* jacobi_solve(const Matrix* A, const Vector* b,
                     double tol, int max_iter);

/* L5: Gauss-Seidel Iteration — uses newest values as soon as available.
 * x_i^{(k+1)} = (b_i - sum_{j<i} a_{ij} x_j^{(k+1)} - sum_{j>i} a_{ij} x_j^{(k)}) / a_{ii}.
 * Converges faster than Jacobi; guaranteed for SPD or diagonally dominant matrices. */
Vector* gauss_seidel_solve(const Matrix* A, const Vector* b,
                           double tol, int max_iter);

/* L5: Successive Over-Relaxation (SOR) — accelerates Gauss-Seidel with relaxation parameter ω.
 * ω ∈ (0,2). ω=1 recovers Gauss-Seidel. ω>1 gives over-relaxation (acceleration).
 * Optimal ω depends on spectral radius of the Jacobi iteration matrix (L4: Young's Theorem). */
Vector* sor_solve(const Matrix* A, const Vector* b,
                  double omega, double tol, int max_iter);

/* L5: Conjugate Gradient — Krylov subspace method for symmetric positive definite systems.
 * Minimizes ||x - x*||_A at each step. Convergence in at most n iterations in exact arithmetic.
 * Derived from the Lanczos process (L4: CG convergence bound — κ(A) determines rate).
 * Complexity: O(n²) per iteration; O(n·nnz) for sparse A. */
Vector* conjugate_gradient(const Matrix* A, const Vector* b,
                           double tol, int max_iter);

/* L5: Preconditioned Conjugate Gradient — uses Jacobi (diagonal) preconditioner M = diag(A).
 * Solves M^{-1}Ax = M^{-1}b. Improves conditioning when κ(M^{-1}A) ≪ κ(A).
 * L8: Incomplete Cholesky preconditioning would further improve convergence. */
Vector* pcg_jacobi(const Matrix* A, const Vector* b,
                   double tol, int max_iter);

/* L5: GMRES (Generalized Minimum Residual) — for non-symmetric systems.
 * Builds Krylov subspace K_m(A, r0) = span{r0, Ar0, ..., A^{m-1}r0}
 * via Arnoldi process and minimizes ||b - Ax_m||_2 over the subspace.
 * Restarted every m steps to bound memory. L4: Convergence depends on field of values. */
Vector* gmres_solve(const Matrix* A, const Vector* b,
                    int restart, double tol, int max_iter);

/* L5: Richardson Iteration — simplest iterative scheme: x^{(k+1)} = x^{(k)} + ω (b - A x^{(k)}).
 * Equivalent to gradient descent with fixed step ω. Converges for ω ∈ (0, 2/ρ(A))
 * where ρ(A) is the spectral radius (L4: Richardson Convergence Theorem). */
Vector* richardson_solve(const Matrix* A, const Vector* b,
                         double omega, double tol, int max_iter);

/* L5: Chebyshev Iteration — accelerates Richardson by using optimal polynomial acceleration.
 * Uses Chebyshev polynomials to minimize error over an interval [λ_min, λ_max] containing
 * the spectrum. Requires estimates of extremal eigenvalues. */
Vector* chebyshev_solve(const Matrix* A, const Vector* b,
                        double lambda_min, double lambda_max,
                        double tol, int max_iter);

/* L5: Symmetric Gauss-Seidel (Symmetric SOR with ω=1) — forward sweep then backward sweep.
 * Yields a symmetric preconditioner; useful as smoother in multigrid methods. */
Vector* symmetric_gauss_seidel(const Matrix* A, const Vector* b,
                                double tol, int max_iter);

/* L5: Weighted Jacobi — damped Jacobi with weight ω ∈ (0,1].
 * Used as smoother in multigrid. ω = 2/3 is standard for Poisson. */
Vector* weighted_jacobi_solve(const Matrix* A, const Vector* b,
                               double omega, double tol, int max_iter);

/* L8: Residual norm history tracking — stores and returns convergence curve.
 * Useful for analyzing solver behavior and convergence rate estimation. */
Vector* iterative_residual_history(const Matrix* A, const Vector* b,
                                    const char* method_name,
                                    double tol, int max_iter,
                                    int* out_iter);

#endif
