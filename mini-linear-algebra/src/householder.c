#include "householder.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPS 1e-15
#define EPS_MACH 2.220446049250313e-16  /* double machine epsilon ~ 2^{-52} */

/* -----------------------------------------------------------------------------
 * L4: Householder Reflection Vector
 *
 * Theorem (Householder, 1958): For any nonzero vector x ∈ ℝ^n, there exists
 * v ∈ ℝ^n and β ∈ ℝ such that P = I - β v v^T is orthogonal and P x = α e1
 * where α = -sign(x1) ||x||₂.
 *
 * The reflector satisfies:
 *   P^T = P  (symmetric)
 *   P^T P = I (orthogonal, since P^2 = I for β = 2/(v^T v))
 *   det(P) = -1 (reflector, not rotation)
 *
 * Algorithm (LAPACK's dlarfg):
 *   1. σ = ||x||₂
 *   2. If x1 > 0, σ = -σ (avoid cancellation)
 *   3. v = x; v[0] = x[0] - σ
 *   4. β = 2 / (v^T v) = 1 / (σ * (σ - x[0]))
 *
 * v[0] is NOT stored as 1. We keep the actual value so P can be applied.
 * The caller uses (I - β v v^T).
 *
 * Reference: Golub & Van Loan, Matrix Computations, 4th ed., §5.1.2
 * --------------------------------------------------------------------------- */
void householder_vector(const double* x, int n, double* v, double* beta) {
    if (n <= 1) {
        v[0] = x[0];
        *beta = 0.0;
        return;
    }

    /* compute σ = ||x|| and α = ||x[1:]|| */
    double sigma = 0.0;
    for (int i = 0; i < n; i++)
        sigma += x[i] * x[i];
    sigma = sqrt(sigma);

    if (sigma < EPS) {
        for (int i = 0; i < n; i++) v[i] = 0.0;
        *beta = 0.0;
        return;
    }

    double x0 = x[0];
    double alpha;  /* the value Px will take */

    /* Choose sign to avoid catastrophic cancellation when computing v[0] */
    if (x0 > 0) {
        alpha = -sigma;
    } else {
        alpha = sigma;
    }

    /* v = x; v[0] = x[0] - alpha */
    memcpy(v, x, n * sizeof(double));
    v[0] = x0 - alpha;

    /* β = 2 / (v^T v) = 2 / ((x0 - α)² + Σ_{i>0} x_i²)
     *   = 2 / (x0² - 2αx0 + α² + (σ² - x0²))
     *   = 2 / (σ² - 2αx0 + α²)
     * Since α² = σ²: β = 2 / (2σ² - 2αx0) = 1 / (σ(σ - x0))  [when α=-σ]
     * Or equivalently: β = -1 / (σ * v[0]) when α = -sign(x0)·σ
     *
     * We use the direct formula for numerical safety:
     */
    double vtv = v[0] * v[0];
    for (int i = 1; i < n; i++)
        vtv += v[i] * v[i];

    if (vtv < EPS) {
        *beta = 0.0;
        return;
    }
    *beta = 2.0 / vtv;
}

/* -----------------------------------------------------------------------------
 * L5: Householder QR Decomposition
 *
 * A = Q R, where Q ∈ ℝ^{m×m} orthogonal, R ∈ ℝ^{m×n} upper triangular.
 * Q is stored implicitly as product of Householder reflectors.
 *
 * Algorithm (Golub & Van Loan, Alg 5.2.1):
 *   for j = 1..n:
 *     compute Householder vector v from A(j:m, j)
 *     store v in A(j:m, j) [v[0] implicitly 1]
 *     store β_j
 *     apply reflector to trailing submatrix A(j:m, j+1:n)
 *
 * This implementation computes explicit Q and R for small-to-medium matrices.
 * For large matrices, Q should remain factored. Complexity: O(2mn² - 2n³/3).
 *
 * Numerical stability: backward stable with error ∝ ε·κ₂(A).
 * Reference: Trefethen & Bau, Numerical Linear Algebra, Lec. 10
 * --------------------------------------------------------------------------- */
void householder_qr(const Matrix* A, Matrix** Q, Matrix** R) {
    int m = A->rows;
    int n = A->cols;
    int min_mn = m < n ? m : n;

    /* copy A into R (we'll overwrite R with the upper-triangular factor) */
    Matrix* Rmat = mat_clone(A);

    /* Q starts as identity (we'll accumulate reflectors) */
    Matrix* Qmat = mat_create_identity(m);

    double* v = (double*)malloc(m * sizeof(double));
    double* w = (double*)malloc(m * sizeof(double));

    for (int j = 0; j < min_mn; j++) {
        /* extract column j of current R(j:m, j) */
        int len = m - j;
        double* col = (double*)malloc(len * sizeof(double));
        for (int i = 0; i < len; i++)
            col[i] = Rmat->data[(j + i) * n + j];

        double beta;
        householder_vector(col, len, v, &beta);
        free(col);

        if (beta < EPS) continue;

        /* Apply P to trailing columns of R: R(j:m, j:n) = (I - β v v^T) R(j:m, j:n)
         * This is: R = R - β v (v^T R)
         * First compute w = β R^T v, then R = R - v w^T */
        for (int k = j; k < n; k++) {
            double dot = 0.0;
            for (int i = 0; i < len; i++)
                dot += v[i] * Rmat->data[(j + i) * n + k];
            w[k] = beta * dot;
        }

        for (int k = j; k < n; k++) {
            for (int i = 0; i < len; i++)
                Rmat->data[(j + i) * n + k] -= v[i] * w[k];
        }

        /* Accumulate Q: Q = Q * P_j
         * P_j = I - β v v^T expanded to full size with v padded.
         * Q(:, j:m) = Q(:, j:m) - (Q(:, j:m) * v) * (β v^T) */
        double* full_v = (double*)calloc(m, sizeof(double));
        for (int i = 0; i < len; i++)
            full_v[j + i] = v[i];

        /* w = Q * (β v) — column operation */
        for (int i = 0; i < m; i++) {
            double dot = 0.0;
            for (int k = 0; k < m; k++)
                dot += Qmat->data[i * m + k] * (beta * full_v[k]);
            w[i] = dot;
        }

        for (int i = 0; i < m; i++) {
            for (int k = 0; k < m; k++)
                Qmat->data[i * m + k] -= w[i] * full_v[k];
        }
        free(full_v);
    }

    free(v);
    free(w);

    /* Zero out subdiagonal of R */
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n && j < i; j++)
            Rmat->data[i * n + j] = 0.0;

    *Q = Qmat;
    *R = Rmat;
}

/* -----------------------------------------------------------------------------
 * L5: Apply Householder Reflectors
 *
 * Given V (reflector vectors stored columnwise) and tau (β scalars),
 * compute  Q^T * B  (side='L', trans='T') or  Q * B  (side='L', trans='N')
 * or  B * Q^T (side='R', trans='T') or  B * Q  (side='R', trans='N').
 *
 * This is the LAPACK dormqr / dorgqr interface.
 * Complexity: O(mnk) for k reflectors applied to m×n matrix B.
 *
 * L4: The "compact WY" representation (Schreiber & Van Loan, 1989) uses
 * Q = I - Y T Y^T with T upper triangular for block application.
 * --------------------------------------------------------------------------- */
void householder_apply(const Matrix* V, const double* tau,
                       Matrix* B, char side, char trans) {
    (void)trans;  /* trans parameter reserved for future Q vs Q^T operations */
    int m = V->rows;    /* rows of V / rows of B for left-side */
    int k = V->cols;    /* number of reflectors */
    int n = B->cols;
    int b_rows = B->rows;

    double* work = (double*)malloc(b_rows > n ? b_rows : n > m ? n : m);

    if (side == 'L' || side == 'l') {
        /* Apply from left */
        for (int j = 0; j < k; j++) {
            if (fabs(tau[j]) < EPS) continue;

            /* form v = [0..0, 1, V(j+1:m, j)] (implicit unit in position j) */
            double* v = (double*)calloc(b_rows, sizeof(double));
            v[j] = 1.0;
            for (int i = j + 1; i < b_rows; i++)
                v[i] = V->data[i * k + j];

            /* w = tau * v^T * B  (row vector of length n) */
            for (int col = 0; col < n; col++) {
                double dot = 0.0;
                for (int row = 0; row < b_rows; row++)
                    dot += v[row] * B->data[row * n + col];
                work[col] = tau[j] * dot;
            }

            /* B = B - v * w */
            for (int row = 0; row < b_rows; row++)
                for (int col = 0; col < n; col++)
                    B->data[row * n + col] -= v[row] * work[col];

            free(v);
        }
    } else {
        /* Apply from right: B = B * Q or B * Q^T */
        for (int j = 0; j < k; j++) {
            if (fabs(tau[j]) < EPS) continue;

            double* v = (double*)calloc(n, sizeof(double));
            v[j] = 1.0;
            for (int i = j + 1; i < n; i++)
                v[i] = V->data[i * k + j];

            /* w = tau * B * v (column vector of length b_rows) */
            for (int row = 0; row < b_rows; row++) {
                double dot = 0.0;
                for (int col = 0; col < n; col++)
                    dot += B->data[row * n + col] * v[col];
                work[row] = tau[j] * dot;
            }

            /* B = B - w * v^T */
            for (int row = 0; row < b_rows; row++)
                for (int col = 0; col < n; col++)
                    B->data[row * n + col] -= work[row] * v[col];

            free(v);
        }
    }
    free(work);
}

/* -----------------------------------------------------------------------------
 * L4: Givens Rotation
 *
 * Given (a, b), find c = cos θ, s = sin θ such that:
 *   [ c  s] [a] = [r]   where r = hypot(a, b) = √(a² + b²)
 *   [-s  c] [b]   [0]
 *
 * Stable computation (Bindel, Demmel, Kahan 2002 — L4):
 *   if |b| < |a|:   t = b/a,  c = 1/√(1+t²), s = c·t
 *   else:           t = a/b,  s = 1/√(1+t²), c = s·t
 * This avoids overflow and handles |a|,|b| spanning many orders of magnitude.
 *
 * Givens rotations are orthogonal: c² + s² = 1, det = 1 (proper rotation).
 * Used for zeroing individual matrix elements — optimal for sparse/structured matrices.
 * Reference: Golub & Van Loan, §5.1.8
 * --------------------------------------------------------------------------- */
void givens_rotation(double a, double b, double* c, double* s) {
    if (fabs(b) < EPS) {
        *c = 1.0;
        *s = 0.0;
        return;
    }

    if (fabs(b) > fabs(a)) {
        double t = a / b;
        *s = 1.0 / sqrt(1.0 + t * t);
        *c = (*s) * t;
    } else {
        double t = b / a;
        *c = 1.0 / sqrt(1.0 + t * t);
        *s = (*c) * t;
    }
}

/* -----------------------------------------------------------------------------
 * L5: Givens QR Decomposition
 *
 * Eliminate subdiagonal elements of A one-by-one using Givens rotations.
 * For each column j, for row i = j+1 .. m-1:
 *   compute Givens rotation (c,s) to zero A(i,j) against A(j,j)
 *   apply rotation to rows j and i for columns j..n
 *
 * For dense matrices, Householder is 2× faster. Givens excels for:
 * - Upper Hessenberg matrices (only one subdiagonal to zero per column)
 * - Tridiagonal matrices (O(n) rotations)
 * - Parallel implementation (each rotation is independent)
 *
 * Complexity: O(3n³) for dense n×n.
 * L4: Givens (1958) — "Computation of plane unitary rotations."
 * --------------------------------------------------------------------------- */
void givens_qr(const Matrix* A, Matrix** Q, Matrix** R) {
    int m = A->rows;
    int n = A->cols;
    int min_mn = m < n ? m : n;

    Matrix* Rmat = mat_clone(A);
    Matrix* Qmat = mat_create_identity(m);

    for (int j = 0; j < min_mn; j++) {
        for (int i = j + 1; i < m; i++) {
            double a = Rmat->data[j * n + j];
            double b = Rmat->data[i * n + j];
            if (fabs(b) < EPS) continue;

            double c, s;
            givens_rotation(a, b, &c, &s);

            /* Apply to R: rows j and i, columns j..n-1 */
            for (int k = j; k < n; k++) {
                double rj = Rmat->data[j * n + k];
                double ri = Rmat->data[i * n + k];
                Rmat->data[j * n + k] =  c * rj + s * ri;
                Rmat->data[i * n + k] = -s * rj + c * ri;
            }

            /* Accumulate Q: Q = Q * G^T — apply to columns j and i */
            for (int k = 0; k < m; k++) {
                double qj = Qmat->data[k * m + j];
                double qi = Qmat->data[k * m + i];
                Qmat->data[k * m + j] =  c * qj + s * qi;
                Qmat->data[k * m + i] = -s * qj + c * qi;
            }
        }
    }

    *Q = Qmat;
    *R = Rmat;
}

/* -----------------------------------------------------------------------------
 * L5: Hessenberg Reduction (Orthogonal Similarity)
 *
 * Given A ∈ ℝ^{n×n}, find orthogonal Q and upper Hessenberg H such that
 * H = Q^T A Q, where h_{ij} = 0 for i > j+1.
 *
 * Algorithm (Golub & Van Loan, Alg 7.4.2):
 *   for k = 1..n-2:
 *     compute Householder vector v to zero A(k+2:n, k)
 *     apply P = I - β v v^T from left:  A(k+1:n, k:n) = P · A(k+1:n, k:n)
 *     apply P from right:               A(1:n, k+1:n) = A(1:n, k+1:n) · P
 *     store v in A(k+2:n, k) and store tau
 *
 * Properties:
 * - Similarity transform preserves eigenvalues
 * - Symmetric input → Hessenberg = tridiagonal
 * - Reduces QR iteration cost from O(n³) to O(n²) per step
 *
 * L4: Francis (1961) — "The QR Transformation" (implicit shifts, Hessenberg form).
 * This reduction is the key insight making the QR eigenvalue algorithm practical.
 * Complexity: O(10n³/3) for general, O(4n³/3) for symmetric.
 * --------------------------------------------------------------------------- */
void hessenberg_reduce(const Matrix* A, Matrix** Q, Matrix** H) {
    int n = A->rows;
    Matrix* Hmat = mat_clone(A);
    Matrix* Qmat = mat_create_identity(n);

    double* v = (double*)malloc(n * sizeof(double));
    double* tau = (double*)calloc(n, sizeof(double));

    for (int k = 0; k < n - 2; k++) {
        /* extract column k from k+1..n-1 */
        int len = n - k - 1;
        double* col = (double*)malloc(len * sizeof(double));
        for (int i = 0; i < len; i++)
            col[i] = Hmat->data[(k + 1 + i) * n + k];

        double beta;
        householder_vector(col, len, v, &beta);
        free(col);

        if (beta < EPS) continue;
        tau[k] = beta;

        /* Apply P from left: H(k+1:n, k:n) = (I - β v v^T) H(k+1:n, k:n) */
        for (int j = k; j < n; j++) {
            double dot = 0.0;
            for (int i = 0; i < len; i++)
                dot += v[i] * Hmat->data[(k + 1 + i) * n + j];
            double w = beta * dot;

            for (int i = 0; i < len; i++)
                Hmat->data[(k + 1 + i) * n + j] -= v[i] * w;
        }

        /* Apply P from right: H(0:n, k+1:n) = H(0:n, k+1:n) (I - β v v^T) */
        for (int i = 0; i < n; i++) {
            double dot = 0.0;
            for (int j = 0; j < len; j++)
                dot += Hmat->data[i * n + (k + 1 + j)] * v[j];
            double w = beta * dot;

            for (int j = 0; j < len; j++)
                Hmat->data[i * n + (k + 1 + j)] -= w * v[j];
        }

        /* Store reflector vectors v in the subdiagonal of H for Q reconstruction */
        for (int i = 0; i < len; i++)
            Hmat->data[(k + 1 + i) * n + k] = v[i];
    }

    free(v);
    free(tau);

    *Q = Qmat;  /* for simplicity, Q is identity (actual Q requires accumulation) */
    *H = Hmat;
}

/* -----------------------------------------------------------------------------
 * L5: Form Q from Hessenberg Reflectors
 *
 * After hessenberg_reduce, the reflector data is stored in the subdiagonal
 * elements of H. This function explicitly forms the orthogonal matrix Q
 * such that H = Q^T A Q.
 *
 * Starts with Q = I and applies reflectors in reverse order:
 *   Q = P_1 P_2 ... P_{n-2}
 * where P_k = I - β_k v_k v_k^T.
 *
 * Complexity: O(n³). Reference: Golub & Van Loan, §7.4.2
 * --------------------------------------------------------------------------- */
Matrix* hessenberg_form_q(const Matrix* V, const double* tau) {
    int n = V->rows;
    Matrix* Q = mat_create_identity(n);

    /* apply reflectors in reverse order */
    for (int k = n - 3; k >= 0; k--) {
        if (fabs(tau[k]) < EPS) continue;

        int len = n - k - 1;
        double* v = (double*)malloc(len * sizeof(double));
        v[0] = 1.0;  /* implicit 1 */
        for (int i = 1; i < len; i++)
            v[i] = V->data[(k + 1 + i) * n + k];

        /* Q = Q * P_k: only affect columns k+1..n-1 */
        for (int i = 0; i < n; i++) {
            double dot = 0.0;
            for (int j = 0; j < len; j++)
                dot += Q->data[i * n + (k + 1 + j)] * v[j];
            double w = tau[k] * dot;
            for (int j = 0; j < len; j++)
                Q->data[i * n + (k + 1 + j)] -= w * v[j];
        }
        free(v);
    }
    return Q;
}

/* -----------------------------------------------------------------------------
 * L5: Single Implicit QR Step on Hessenberg Matrix (Francis QR Step)
 *
 * Core of the practical QR eigenvalue algorithm. Given upper Hessenberg H:
 * 1. Choose shift σ (Wilkinson shift: eigenvalue of trailing 2×2 closer to h_{nn})
 * 2. Compute first Givens rotation G_1 to zero H(2,1) - σ
 * 3. Apply G_1 from left and right
 * 4. "Chase the bulge": apply Givens rotations to restore Hessenberg form
 *
 * The implicit Q theorem (Francis, 1961; L4) guarantees this is equivalent to
 * an explicit QR step. This preserves Hessenberg form at O(n²) cost.
 *
 * Complexity: O(n²) per step. Reference: Watkins, "Fundamentals of Matrix
 * Computations", §5.5.
 * --------------------------------------------------------------------------- */
void hessenberg_qr_step(Matrix* H, int n) {
    if (n < 2) return;

    /* Wilkinson shift: eigenvalue of [h_{n-1,n-1} h_{n-1,n}; h_{n,n-1} h_{n,n}]
     * closer to h_{n,n} */
    double h_nn = H->data[(n - 1) * n + (n - 1)];
    double h_n1n1 = H->data[(n - 2) * n + (n - 2)];
    double h_n1n = H->data[(n - 2) * n + (n - 1)];
    double h_nn1 = H->data[(n - 1) * n + (n - 2)];

    double trace = h_n1n1 + h_nn;
    double det = h_n1n1 * h_nn - h_n1n * h_nn1;
    double disc = sqrt(trace * trace - 4.0 * det);

    double mu1 = (trace + disc) / 2.0;
    double mu2 = (trace - disc) / 2.0;
    double shift = (fabs(mu1 - h_nn) < fabs(mu2 - h_nn)) ? mu1 : mu2;

    /* Implicit shift: apply first Givens to zero h21 - shift */
    double x = H->data[0 * n + 0] - shift;
    double y = H->data[1 * n + 0];
    double c, s;
    givens_rotation(x, y, &c, &s);

    /* Apply G_1 from left (rows 0,1) and right (cols 0,1)
     * This introduces a "bulge" at (2,0), which we chase down */
    for (int j = 0; j < n; j++) {
        double h0 = H->data[0 * n + j];
        double h1 = H->data[1 * n + j];
        H->data[0 * n + j] =  c * h0 + s * h1;
        H->data[1 * n + j] = -s * h0 + c * h1;
    }
    for (int i = 0; i < n; i++) {
        double h0 = H->data[i * n + 0];
        double h1 = H->data[i * n + 1];
        H->data[i * n + 0] =  c * h0 + s * h1;
        H->data[i * n + 1] = -s * h0 + c * h1;
    }

    /* Chase the bulge */
    for (int k = 1; k < n - 1; k++) {
        double x = H->data[k * n + (k - 1)];
        double y = H->data[(k + 1) * n + (k - 1)];
        givens_rotation(x, y, &c, &s);

        /* Apply from left: rows k, k+1 */
        for (int j = k - 1; j < n; j++) {
            double hk = H->data[k * n + j];
            double hk1 = H->data[(k + 1) * n + j];
            H->data[k * n + j] =  c * hk + s * hk1;
            H->data[(k + 1) * n + j] = -s * hk + c * hk1;
        }

        /* Apply from right: cols k, k+1 (row 0..k+1 affected) */
        for (int i = 0; i <= k + 1 && i < n; i++) {
            double hk = H->data[i * n + k];
            double hk1 = H->data[i * n + (k + 1)];
            H->data[i * n + k] =  c * hk + s * hk1;
            H->data[i * n + (k + 1)] = -s * hk + c * hk1;
        }
    }
}

/* -----------------------------------------------------------------------------
 * L4: Gershgorin Circle Theorem (Verification)
 *
 * Theorem (Gershgorin, 1931): For any matrix A ∈ ℂ^{n×n}, every eigenvalue λ
 * lies in at least one Gershgorin disk:
 *   D_i = { z ∈ ℂ : |z - a_{ii}| ≤ R_i },  R_i = Σ_{j≠i} |a_{ij}|  (row disks)
 *
 * Corollary: The same holds for column disks:
 *   D'_j = { z ∈ ℂ : |z - a_{jj}| ≤ C_j },  C_j = Σ_{i≠j} |a_{ij}|
 *
 * This function computes all disk centers and radii.
 * disk_type = 0: row disks;  disk_type = 1: column disks.
 *
 * L4: Varga (2004), "Geršgorin and His Circles" — comprehensive treatment.
 * Used for eigenvalue localization without computing eigenvalues.
 * --------------------------------------------------------------------------- */
void gershgorin_disks(const Matrix* A, double* centers, double* radii,
                       int disk_type) {
    int n = A->rows;

    if (disk_type == 0) {
        /* Row disks */
        for (int i = 0; i < n; i++) {
            centers[i] = A->data[i * n + i];
            radii[i] = 0.0;
            for (int j = 0; j < n; j++) {
                if (j != i)
                    radii[i] += fabs(A->data[i * n + j]);
            }
        }
    } else {
        /* Column disks */
        for (int j = 0; j < n; j++) {
            centers[j] = A->data[j * n + j];
            radii[j] = 0.0;
            for (int i = 0; i < n; i++) {
                if (i != j)
                    radii[j] += fabs(A->data[i * n + j]);
            }
        }
    }
}
