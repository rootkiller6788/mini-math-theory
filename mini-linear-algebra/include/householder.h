#ifndef HOUSEHOLDER_H
#define HOUSEHOLDER_H

#include "matrix.h"

/* L4: Householder Reflection — compute reflector H = I - β v v^T such that
 * H * x = ±||x|| e1. Stores v (with v[0]=1 implicit) and scalar β in-place.
 * Theorem (Householder, 1958): For any vector x ∈ ℝ^n, there exists v, β
 * such that (I - β v v^T) x = ||x|| e1. Numerical stability from avoiding
 * cancellation in the sign choice. */
void householder_vector(const double* x, int n, double* v, double* beta);

/* L5: Householder QR Decomposition — compute Q orthogonal and R upper triangular
 * such that A = Q R. More numerically stable than classical Gram-Schmidt.
 * Q is stored implicitly as product of Householder reflectors; R is explicit.
 * Complexity: O(mn²) for m×n matrix. */
void householder_qr(const Matrix* A, Matrix** Q, Matrix** R);

/* L5: Apply Householder Reflectors to Compute Q^T * B or Q * B.
 * Given Householder vectors V and scalars tau, compute the product.
 * side='L' for Q^T B, side='R' for B Q. trans='T' for transpose. */
void householder_apply(const Matrix* V, const double* tau,
                       Matrix* B, char side, char trans);

/* L4: Givens Rotation — compute (c, s) such that
 * [ c  s] [a]   [r]
 * [-s  c] [b] = [0]
 * with r = hypot(a, b). Stable computation avoids overflow (L4: Bindel et al.). */
void givens_rotation(double a, double b, double* c, double* s);

/* L5: Givens QR Decomposition — eliminate subdiagonal elements one at a time.
 * More efficient than Householder for matrices with special structure
 * (e.g., upper Hessenberg, tridiagonal). Complexity: O(3n³) for dense. */
void givens_qr(const Matrix* A, Matrix** Q, Matrix** R);

/* L5: Hessenberg Reduction — reduce general matrix A to upper Hessenberg form H
 * via orthogonal similarity: H = Q^T A Q, where h_{ij} = 0 for i > j+1.
 * Essential preprocessing for the QR eigenvalue algorithm (L4: Francis, 1961).
 * Complexity: O(n³). */
void hessenberg_reduce(const Matrix* A, Matrix** Q, Matrix** H);

/* L5: Extract Hessenberg matrix Q from accumulated Householder reflectors.
 * After hessenberg_reduce, Q is stored implicitly; this makes it explicit. */
Matrix* hessenberg_form_q(const Matrix* V, const double* tau);

/* L5: Implicit QR Step on Hessenberg Matrix — Francis QR step with Wilkinson shift.
 * Applies one QR iteration with implicit shifts to a Hessenberg matrix H,
 * preserving the Hessenberg form. Core of the practical QR eigenvalue algorithm. */
void hessenberg_qr_step(Matrix* H, int n);

/* L4: Gershgorin Circle Theorem (verification) — for every eigenvalue λ of A,
 * there exists i such that |λ - a_{ii}| ≤ Σ_{j≠i} |a_{ij}| (row disks).
 * Also computes column disks. Returns the union bound. */
void gershgorin_disks(const Matrix* A, double* centers, double* radii,
                       int disk_type);

#endif
