#ifndef SUBSPACE_H
#define SUBSPACE_H

#include "matrix.h"
#include <stdbool.h>

/* L1: Linear Independence Check — determine if columns of A are linearly independent.
 * Returns true if rank(A) == n (full column rank). Uses QR with column pivoting
 * to handle near-rank-deficiency numerically. */
bool is_linearly_independent(const Matrix* A);

/* L1: Span Check — determine if vector b lies in the column space (span) of A.
 * Equivalent to checking if Ax = b has a solution. Uses least-squares residual. */
bool is_in_span(const Matrix* A, const Vector* b);

/* L2: Orthogonal Projection — project vector b onto the column space of A.
 * P_A b = A (A^T A)^{-1} A^T b. Uses QR for numerical stability.
 * This is the fundamental operation in least-squares (L4: Projection Theorem). */
Vector* orthogonal_project(const Matrix* A, const Vector* b);

/* L2: Orthogonal Complement — compute a basis for the nullspace of A^T,
 * i.e., vectors w such that w^T A = 0 (left nullspace).
 * dim(N(A^T)) = m - rank(A). Fundamental Theorem of Linear Algebra. */
Matrix* orthogonal_complement(const Matrix* A);

/* L5: Classical Gram-Schmidt — orthogonalize columns of A to produce Q.
 * Q has orthonormal columns spanning the same space as A. R = Q^T A.
 * Numerically unstable for near-dependent columns; use Modified GS instead. */
void gram_schmidt_classical(const Matrix* A, Matrix** Q, Matrix** R);

/* L5: Modified Gram-Schmidt — numerically stable variant.
 * Performs projection and subtraction column-by-column with updated vectors.
 * Loss of orthogonality ∝ κ(A) * ε vs ∝ κ²(A) * ε for classical GS.
 * L4: Björck (1967) error analysis of MGS. */
void gram_schmidt_modified(const Matrix* A, Matrix** Q, Matrix** R);

/* L2: Basis Extraction — given a set of vectors (columns of A), extract
 * a maximal linearly independent subset. Returns matrix whose columns
 * form a basis for col(A). Uses rank-revealing QR. */
Matrix* extract_basis(const Matrix* A);

/* L2: Subspace Distance — principal angles between two subspaces.
 * col(A) and col(B). Returns the largest principal angle (in radians),
 * a measure of subspace separation (L4: Jordan, 1875). */
double subspace_angle(const Matrix* A, const Matrix* B);

/* L4: Fundamental Subspaces — compute all four fundamental subspaces
 * for matrix A: column space C(A), row space C(A^T), nullspace N(A),
 * left nullspace N(A^T). Prints dimensions verifying:
 * dim(C(A)) + dim(N(A^T)) = m,  dim(C(A^T)) + dim(N(A)) = n.
 * (L4: Fundamental Theorem of Linear Algebra — Strang, 1993) */
void fundamental_subspaces(const Matrix* A);

/* L2: Orthogonalize a vector against a set of basis vectors.
 * Given basis Q (columns orthonormal) and vector w, returns w_perp
 * orthogonal to span(Q). Uses modified Gram-Schmidt step. */
Vector* orthogonalize_against(const Matrix* Q, const Vector* w);

#endif
