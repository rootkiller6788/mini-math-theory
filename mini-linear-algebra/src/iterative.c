#include "iterative.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPS 1e-15

/* -----------------------------------------------------------------------------
 * L5: Jacobi Iteration
 * Splitting A = D + L + U where D = diag(A), L = strict lower, U = strict upper.
 * x^{(k+1)} = D^{-1} (b - (L+U) x^{(k)})
 * Component-wise: x_i^{(k+1)} = (b_i - Σ_{j≠i} a_{ij} x_j^{(k)}) / a_{ii}
 *
 * Convergence Theorem (L4): Jacobi converges for any x0 iff ρ(D^{-1}(L+U)) < 1.
 * Sufficient condition: strict diagonal dominance or irreducibly diagonally dominant.
 *
 * Complexity: O(n²) per iteration, O(κ log(1/ε)) total for SPD matrices.
 * Reference: Saad, Iterative Methods for Sparse Linear Systems, 2nd ed., Ch.4
 * --------------------------------------------------------------------------- */
Vector* jacobi_solve(const Matrix* A, const Vector* b, double tol, int max_iter) {
    int n = A->rows;
    Vector* x = vec_create(n);         /* initial guess x0 = 0 */
    Vector* x_new = vec_create(n);
    double* D_inv = (double*)malloc(n * sizeof(double));

    /* precompute D^{-1} */
    for (int i = 0; i < n; i++) {
        double aii = A->data[i * n + i];
        if (fabs(aii) < 1e-15) {       /* zero diagonal — Jacobi fails */
            free(D_inv);
            vec_free(x_new);
            vec_free(x);
            return NULL;
        }
        D_inv[i] = 1.0 / aii;
    }

    for (int iter = 0; iter < max_iter; iter++) {
        for (int i = 0; i < n; i++) {
            double sum = b->data[i];
            for (int j = 0; j < n; j++) {
                if (j != i)
                    sum -= A->data[i * n + j] * x->data[j];
            }
            x_new->data[i] = sum * D_inv[i];
        }

        /* check convergence: ||x_new - x||_∞ < tol ||x_new||_∞ */
        double diff_norm = 0.0, x_norm = 0.0;
        for (int i = 0; i < n; i++) {
            double d = fabs(x_new->data[i] - x->data[i]);
            if (d > diff_norm) diff_norm = d;
            double xn = fabs(x_new->data[i]);
            if (xn > x_norm) x_norm = xn;
        }
        if (x_norm < EPS) x_norm = 1.0;
        if (diff_norm < tol * x_norm) {
            memcpy(x->data, x_new->data, n * sizeof(double));
            break;
        }
        memcpy(x->data, x_new->data, n * sizeof(double));
    }

    vec_free(x_new);
    free(D_inv);
    return x;
}

/* -----------------------------------------------------------------------------
 * L5: Gauss-Seidel Iteration
 * Uses newest values immediately: x^{(k+1)} = (D+L)^{-1} (b - U x^{(k)})
 * Component-wise forward sweep uses x_j^{(k+1)} for j < i.
 *
 * For SPD matrices, GS converges for any x0. For strictly diagonally dominant,
 * GS converges and is asymptotically twice as fast as Jacobi (L4: Varga, 1962).
 *
 * Complexity: O(n²) per iteration; typically half the iterations of Jacobi.
 * --------------------------------------------------------------------------- */
Vector* gauss_seidel_solve(const Matrix* A, const Vector* b,
                            double tol, int max_iter) {
    int n = A->rows;
    Vector* x = vec_create(n);         /* initial guess x0 = 0 */

    for (int iter = 0; iter < max_iter; iter++) {
        double max_diff = 0.0, max_x = 0.0;

        for (int i = 0; i < n; i++) {
            double sum = b->data[i];
            for (int j = 0; j < i; j++)
                sum -= A->data[i * n + j] * x->data[j];  /* use updated values */
            for (int j = i + 1; j < n; j++)
                sum -= A->data[i * n + j] * x->data[j];  /* use old values */
            double x_new = sum / A->data[i * n + i];

            double diff = fabs(x_new - x->data[i]);
            if (diff > max_diff) max_diff = diff;
            if (fabs(x_new) > max_x) max_x = fabs(x_new);

            x->data[i] = x_new;  /* in-place update */
        }

        if (max_x < EPS) max_x = 1.0;
        if (max_diff < tol * max_x)
            break;
    }
    return x;
}

/* -----------------------------------------------------------------------------
 * L5: Successive Over-Relaxation (SOR)
 * Introduces relaxation parameter ω. For ω = 1, recovers Gauss-Seidel.
 * x_i^{(k+1)} = (1-ω) x_i^{(k)} + (ω/a_{ii})(b_i - Σ_{j<i} a_{ij} x_j^{(k+1)} - Σ_{j>i} a_{ij} x_j^{(k)})
 *
 * Convergence requires 0 < ω < 2 (L4: Ostrowski-Reich theorem).
 * Optimal ω_opt = 2 / (1 + √(1 - ρ²)) where ρ = ρ(D^{-1}(L+U)) for consistently
 * ordered matrices (L4: Young's Theorem, 1950).
 *
 * Complexity: O(n²) per iteration. Can significantly reduce iteration count.
 * Reference: Young, "Iterative Solution of Large Linear Systems", 1971.
 * --------------------------------------------------------------------------- */
Vector* sor_solve(const Matrix* A, const Vector* b,
                  double omega, double tol, int max_iter) {
    int n = A->rows;
    Vector* x = vec_create(n);
    double one_minus_omega = 1.0 - omega;

    for (int iter = 0; iter < max_iter; iter++) {
        double max_diff = 0.0, max_x = 0.0;

        for (int i = 0; i < n; i++) {
            double sum_gs = b->data[i];
            for (int j = 0; j < i; j++)
                sum_gs -= A->data[i * n + j] * x->data[j];
            for (int j = i + 1; j < n; j++)
                sum_gs -= A->data[i * n + j] * x->data[j];
            double gs_update = sum_gs / A->data[i * n + i];

            double x_new = one_minus_omega * x->data[i] + omega * gs_update;

            double diff = fabs(x_new - x->data[i]);
            if (diff > max_diff) max_diff = diff;
            if (fabs(x_new) > max_x) max_x = fabs(x_new);

            x->data[i] = x_new;
        }

        if (max_x < EPS) max_x = 1.0;
        if (max_diff < tol * max_x)
            break;
    }
    return x;
}

/* -----------------------------------------------------------------------------
 * L5: Conjugate Gradient (CG) — Hestenes & Stiefel (1952)
 * Solves Ax = b for A symmetric positive definite.
 *
 * Algorithm:
 *   r0 = b - A x0;  p0 = r0
 *   for k = 0,1,...:
 *     α_k = (r_k, r_k) / (p_k, A p_k)
 *     x_{k+1} = x_k + α_k p_k
 *     r_{k+1} = r_k - α_k A p_k
 *     β_k = (r_{k+1}, r_{k+1}) / (r_k, r_k)
 *     p_{k+1} = r_{k+1} + β_k p_k
 *
 * Properties:
 * - (r_i, r_j) = 0 for i ≠ j (orthogonal residuals)
 * - (p_i, A p_j) = 0 for i ≠ j (A-conjugate directions)
 * - ||x_k - x*||_A ≤ 2 ((√κ - 1)/(√κ + 1))^k ||x0 - x*||_A (L4: CG convergence)
 * - Converges in ≤ n steps in exact arithmetic
 *
 * κ = λ_max / λ_min is the condition number of A.
 * Complexity: O(n²) per iteration (1 mat-vec, 2 dot products, 3 axpys).
 * Reference: Trefethen & Bau, Numerical Linear Algebra, Lec. 38
 * --------------------------------------------------------------------------- */
Vector* conjugate_gradient(const Matrix* A, const Vector* b,
                            double tol, int max_iter) {
    int n = A->rows;
    Vector* x = vec_create(n);
    Vector* r = vec_clone(b);             /* r0 = b - A*0 = b */
    Vector* p = vec_clone(r);             /* p0 = r0 */
    Vector* Ap = vec_create(n);

    double rtr_old = vec_dot(r, r);
    double b_norm = vec_norm(b);
    if (b_norm < EPS) b_norm = 1.0;

    for (int k = 0; k < max_iter && k < n; k++) {
        /* Ap = A * p */
        Vector* temp = mat_vec_mul(A, p);
        memcpy(Ap->data, temp->data, n * sizeof(double));
        vec_free(temp);

        double pAp = vec_dot(p, Ap);
        if (fabs(pAp) < 1e-30) break;     /* breakdown — singular A? */

        double alpha = rtr_old / pAp;

        /* x = x + α p */
        for (int i = 0; i < n; i++)
            x->data[i] += alpha * p->data[i];

        /* r = r - α Ap */
        for (int i = 0; i < n; i++)
            r->data[i] -= alpha * Ap->data[i];

        double rtr_new = vec_dot(r, r);

        /* convergence check: ||r|| / ||b|| < tol */
        if (sqrt(rtr_new) / b_norm < tol)
            break;

        double beta = rtr_new / rtr_old;

        /* p = r + β p */
        for (int i = 0; i < n; i++)
            p->data[i] = r->data[i] + beta * p->data[i];

        rtr_old = rtr_new;
    }

    vec_free(r);
    vec_free(p);
    vec_free(Ap);
    return x;
}

/* -----------------------------------------------------------------------------
 * L5: Preconditioned CG with Jacobi Preconditioner
 * M = diag(A).  M^{-1/2} A M^{-1/2} has unit diagonal.
 *
 * For well-conditioned SPD matrices, Jacobi preconditioning reduces κ
 * significantly when A has varying diagonal magnitudes.
 *
 * L8: More advanced preconditioners (incomplete Cholesky, multigrid, domain
 * decomposition) can achieve mesh-independent convergence for PDE problems.
 *
 * Implementation uses split preconditioning: solves M^{-1/2} A M^{-1/2} y = M^{-1/2} b,
 * then x = M^{-1/2} y. This preserves symmetry.
 * --------------------------------------------------------------------------- */
Vector* pcg_jacobi(const Matrix* A, const Vector* b,
                    double tol, int max_iter) {
    int n = A->rows;
    Vector* x = vec_create(n);
    Vector* r = vec_clone(b);
    Vector* z = vec_create(n);            /* M^{-1} r */
    Vector* p = vec_create(n);
    Vector* Ap = vec_create(n);

    /* initial preconditioned residual z0 = M^{-1} r0 */
    for (int i = 0; i < n; i++) {
        double diag = A->data[i * n + i];
        z->data[i] = r->data[i] / diag;
    }
    memcpy(p->data, z->data, n * sizeof(double));

    double rz_old = vec_dot(r, z);
    double b_norm = vec_norm(b);
    if (b_norm < EPS) b_norm = 1.0;

    for (int k = 0; k < max_iter && k < n; k++) {
        Vector* temp = mat_vec_mul(A, p);
        memcpy(Ap->data, temp->data, n * sizeof(double));
        vec_free(temp);

        double pAp = vec_dot(p, Ap);
        if (fabs(pAp) < 1e-30) break;

        double alpha = rz_old / pAp;

        for (int i = 0; i < n; i++)
            x->data[i] += alpha * p->data[i];
        for (int i = 0; i < n; i++)
            r->data[i] -= alpha * Ap->data[i];

        /* preconditioned residual */
        for (int i = 0; i < n; i++) {
            double diag = A->data[i * n + i];
            z->data[i] = r->data[i] / diag;
        }

        double rz_new = vec_dot(r, z);
        if (sqrt(rz_new) / b_norm < tol) break;

        double beta = rz_new / rz_old;
        for (int i = 0; i < n; i++)
            p->data[i] = z->data[i] + beta * p->data[i];
        rz_old = rz_new;
    }

    vec_free(r);
    vec_free(z);
    vec_free(p);
    vec_free(Ap);
    return x;
}

/* -----------------------------------------------------------------------------
 * L5: GMRES(m) — Saad & Schultz (1986)
 * For general non-symmetric A. Minimizes ||b - A x_m||_2 over Krylov subspace
 * K_m(A, r0). Restarted every m steps.
 *
 * Builds Arnoldi factorization: A V_m = V_{m+1} H̄_m
 * Then solves least-squares: min_y ||β e1 - H̄_m y||_2
 * where β = ||r0||, then x = x0 + V_m y.
 *
 * Complexity: O(m²n) per cycle. Storage: O(mn) for V_m.
 * Restart m trades convergence speed (larger m) vs memory (smaller m).
 *
 * L4: Convergence theory — depends on field of values and pseudospectra.
 * For normal matrices, convergence bound is in terms of the distance of the
 * spectrum to the origin.  Reference: Saad, §6.5.
 * --------------------------------------------------------------------------- */
Vector* gmres_solve(const Matrix* A, const Vector* b,
                     int restart, double tol, int max_iter) {
    int n = A->rows;
    Vector* x = vec_create(n);
    Vector* r = vec_clone(b);
    double beta = vec_norm(r);

    double b_norm = beta;
    if (b_norm < EPS) b_norm = 1.0;

    if (beta < tol * b_norm) {
        vec_free(r);
        return x;
    }

    /* allocate Krylov basis V: (restart+1) × n */
    double** V = (double**)malloc((restart + 1) * sizeof(double*));
    for (int i = 0; i <= restart; i++)
        V[i] = (double*)calloc(n, sizeof(double));

    /* Hessenberg matrix H̄: (restart+1) × restart (column-major) */
    double* H = (double*)calloc((restart + 1) * restart, sizeof(double));

    double* g = (double*)calloc(restart + 1, sizeof(double));  /* Givens rotation cos */
    double* s = (double*)calloc(restart + 1, sizeof(double));  /* Givens rotation sin */
    double* cs = (double*)calloc(restart + 1, sizeof(double)); /* RHS for triangular system */

    int total_iters = 0;

    while (total_iters < max_iter) {
        /* compute r = b - A*x and v1 = r / beta (restarted residual) */
        Vector* Ax = mat_vec_mul(A, x);
        for (int i = 0; i < n; i++)
            r->data[i] = b->data[i] - Ax->data[i];
        vec_free(Ax);
        beta = vec_norm(r);
        if (beta < tol * b_norm) break;

        for (int i = 0; i < n; i++)
            V[0][i] = r->data[i] / beta;

        memset(cs, 0, (restart + 1) * sizeof(double));
        memset(H, 0, (restart + 1) * restart * sizeof(double));
        cs[0] = beta;

        int j;
        for (j = 0; j < restart; j++) {
            /* w = A * v_j */
            Vector* v = vec_create(n);
            for (int i = 0; i < n; i++) v->data[i] = V[j][i];
            Vector* w = mat_vec_mul(A, v);
            vec_free(v);

            /* Arnoldi: orthogonalize w against V_0,...,V_j */
            for (int i = 0; i <= j; i++) {
                double dot = 0.0;
                for (int k = 0; k < n; k++)
                    dot += V[i][k] * w->data[k];
                H[i * restart + j] = dot;
                for (int k = 0; k < n; k++)
                    w->data[k] -= dot * V[i][k];
            }

            double h_next = 0.0;
            for (int k = 0; k < n; k++)
                h_next += w->data[k] * w->data[k];
            h_next = sqrt(h_next);
            H[(j + 1) * restart + j] = h_next;

            if (h_next > EPS) {
                for (int k = 0; k < n; k++)
                    V[j + 1][k] = w->data[k] / h_next;
            }
            vec_free(w);

            /* apply previous Givens rotations to column j of H */
            for (int i = 0; i < j; i++) {
                double tmp = g[i] * H[i * restart + j] + s[i] * H[(i + 1) * restart + j];
                H[(i + 1) * restart + j] = -s[i] * H[i * restart + j] + g[i] * H[(i + 1) * restart + j];
                H[i * restart + j] = tmp;
            }

            /* compute Givens rotation to zero H(j+1,j) */
            double h_jj = H[j * restart + j];
            double h_j1j = H[(j + 1) * restart + j];
            double rho = sqrt(h_jj * h_jj + h_j1j * h_j1j);
            if (rho < EPS) rho = EPS;
            g[j] = h_jj / rho;
            s[j] = h_j1j / rho;
            H[j * restart + j] = rho;
            H[(j + 1) * restart + j] = 0.0;

            /* apply to RHS */
            double tmp = g[j] * cs[j] + s[j] * cs[j + 1];
            cs[j + 1] = -s[j] * cs[j] + g[j] * cs[j + 1];
            cs[j] = tmp;

            if (fabs(cs[j + 1]) < tol * b_norm)
                break;
        }

        int m = (j < restart) ? (j + 1) : restart;

        /* solve R y = cs (back substitution using triangular H) */
        double* y = (double*)calloc(m, sizeof(double));
        for (int i = m - 1; i >= 0; i--) {
            double sum = cs[i];
            for (int k = i + 1; k < m; k++)
                sum -= H[i * restart + k] * y[k];
            if (fabs(H[i * restart + i]) > EPS)
                y[i] = sum / H[i * restart + i];
        }

        /* x = x + V_m * y */
        for (int i = 0; i < m; i++) {
            for (int k = 0; k < n; k++)
                x->data[k] += y[i] * V[i][k];
        }

        free(y);
        total_iters += m + 1;

        if (m < restart) break;  /* converged within cycle */
    }

    for (int i = 0; i <= restart; i++) free(V[i]);
    free(V);
    free(H);
    free(g);
    free(s);
    free(cs);
    vec_free(r);
    return x;
}

/* -----------------------------------------------------------------------------
 * L5: Richardson Iteration
 * x^{(k+1)} = x^{(k)} + ω (b - A x^{(k)})
 *
 * This is gradient descent with fixed step size for f(x) = ½ x^T A x - b^T x.
 * Converges iff 0 < ω < 2/λ_max(A) for SPD A.
 * Optimal ω = 2/(λ_min + λ_max) minimizes the spectral radius of I - ωA.
 *
 * L4: Richardson convergence theorem (1910) — one of the earliest iterative methods.
 * Complexity: O(n²) per iteration. Convergence rate: ρ(I - ωA)^k.
 * --------------------------------------------------------------------------- */
Vector* richardson_solve(const Matrix* A, const Vector* b,
                          double omega, double tol, int max_iter) {
    int n = A->rows;
    Vector* x = vec_create(n);
    Vector* r = vec_create(n);

    for (int iter = 0; iter < max_iter; iter++) {
        /* r = b - A x */
        Vector* Ax = mat_vec_mul(A, x);
        for (int i = 0; i < n; i++)
            r->data[i] = b->data[i] - Ax->data[i];
        vec_free(Ax);

        double r_norm = vec_norm(r);
        if (r_norm < tol) break;

        /* x = x + ω r */
        for (int i = 0; i < n; i++)
            x->data[i] += omega * r->data[i];
    }

    vec_free(r);
    return x;
}

/* -----------------------------------------------------------------------------
 * L5: Chebyshev Iteration
 * Uses Chebyshev polynomials to accelerate Richardson by constructing an
 * optimal polynomial p_k(A) that minimizes the error over [λ_min, λ_max].
 *
 * Recurrence: x^{(k+1)} = x^{(k)} + α_k (x^{(k)} - x^{(k-1)}) + β_k r^{(k)}
 * where α_k, β_k are derived from Chebyshev polynomial coefficients.
 *
 * Requires estimates of λ_min, λ_max (e.g., from Gershgorin or a few power iters).
 * L4: The Chebyshev semi-iterative method (Golub & Varga, 1961).
 * Optimal for eigenvalue distribution in a real interval.
 * --------------------------------------------------------------------------- */
Vector* chebyshev_solve(const Matrix* A, const Vector* b,
                         double lambda_min, double lambda_max,
                         double tol, int max_iter) {
    int n = A->rows;
    Vector* x = vec_create(n);
    Vector* r = vec_create(n);
    Vector* x_prev = vec_create(n);

    double d = (lambda_max + lambda_min) / 2.0;
    double c = (lambda_max - lambda_min) / 2.0;
    if (c < EPS) c = 1.0;  /* avoid division by zero */

    for (int iter = 0; iter < max_iter; iter++) {
        /* r = b - A x */
        Vector* Ax = mat_vec_mul(A, x);
        for (int i = 0; i < n; i++)
            r->data[i] = b->data[i] - Ax->data[i];
        vec_free(Ax);

        double r_norm = vec_norm(r);
        if (r_norm < tol) break;

        double theta;
        if (iter == 0) {
            theta = 1.0;
        } else {
            double rho = c / d;
            double prev_theta = (iter == 1) ? 1.0 : (1.0 + rho * rho / 4.0);
            theta = 1.0 / (1.0 - rho * rho / (4.0 * prev_theta));
        }

        /* update x */
        double* x_temp = (double*)malloc(n * sizeof(double));
        for (int i = 0; i < n; i++) {
            double dx = (iter == 0) ? 0.0 : (x->data[i] - x_prev->data[i]);
            x_temp[i] = x->data[i] + (theta - 1.0) * dx + theta * r->data[i] / d;
        }

        memcpy(x_prev->data, x->data, n * sizeof(double));
        memcpy(x->data, x_temp, n * sizeof(double));
        free(x_temp);
    }

    vec_free(r);
    vec_free(x_prev);
    return x;
}

/* -----------------------------------------------------------------------------
 * L5: Symmetric Gauss-Seidel (SSOR with ω=1)
 * Forward sweep (standard GS) followed by backward sweep.
 * Produces a symmetric preconditioner: M = (D+L) D^{-1} (D+U).
 * Used as smoother in multigrid methods (L8).
 *
 * Backward sweep: x_i = (b_i - Σ_{j>i} a_{ij} x_j^{(new)} - Σ_{j<i} a_{ij} x_j^{(old)}) / a_{ii}
 * --------------------------------------------------------------------------- */
Vector* symmetric_gauss_seidel(const Matrix* A, const Vector* b,
                                double tol, int max_iter) {
    int n = A->rows;
    Vector* x = vec_create(n);

    for (int iter = 0; iter < max_iter; iter++) {
        double max_diff = 0.0, max_x = 0.0;

        /* forward sweep (standard GS) */
        for (int i = 0; i < n; i++) {
            double sum = b->data[i];
            for (int j = 0; j < i; j++)
                sum -= A->data[i * n + j] * x->data[j];
            for (int j = i + 1; j < n; j++)
                sum -= A->data[i * n + j] * x->data[j];
            double x_new = sum / A->data[i * n + i];
            x->data[i] = x_new;
        }

        /* backward sweep */
        for (int i = n - 1; i >= 0; i--) {
            double sum = b->data[i];
            for (int j = 0; j < i; j++)
                sum -= A->data[i * n + j] * x->data[j];
            for (int j = i + 1; j < n; j++)
                sum -= A->data[i * n + j] * x->data[j];
            double x_new = sum / A->data[i * n + i];

            double diff = fabs(x_new - x->data[i]);
            if (diff > max_diff) max_diff = diff;
            if (fabs(x_new) > max_x) max_x = fabs(x_new);

            x->data[i] = x_new;
        }

        if (max_x < EPS) max_x = 1.0;
        if (max_diff < tol * max_x) break;
    }
    return x;
}

/* -----------------------------------------------------------------------------
 * L5: Weighted Jacobi — damped Jacobi with weight ω ∈ (0, 1].
 * x^{(k+1)} = (1-ω) x^{(k)} + ω D^{-1} (b - (L+U) x^{(k)})
 *
 * Standard multigrid smoother for Poisson equation. ω = 2/3 is optimal
 * for the 2D Laplace operator, damping the high-frequency error components.
 *
 * L4: For SPD A, weighted Jacobi converges if 0 < ω < 2/ρ(D^{-1}A).
 * L8: Multigrid methods use weighted Jacobi as a smoother.
 * Reference: Briggs, Henson & McCormick, "A Multigrid Tutorial", §2.1
 * --------------------------------------------------------------------------- */
Vector* weighted_jacobi_solve(const Matrix* A, const Vector* b,
                               double omega, double tol, int max_iter) {
    int n = A->rows;
    Vector* x = vec_create(n);
    Vector* x_new = vec_create(n);
    double one_minus_omega = 1.0 - omega;

    for (int iter = 0; iter < max_iter; iter++) {
        for (int i = 0; i < n; i++) {
            double sum = b->data[i];
            for (int j = 0; j < n; j++) {
                if (j != i)
                    sum -= A->data[i * n + j] * x->data[j];
            }
            double j_update = sum / A->data[i * n + i];
            x_new->data[i] = one_minus_omega * x->data[i] + omega * j_update;
        }

        double diff_norm = 0.0, x_norm = 0.0;
        for (int i = 0; i < n; i++) {
            double d = fabs(x_new->data[i] - x->data[i]);
            if (d > diff_norm) diff_norm = d;
            double xn = fabs(x_new->data[i]);
            if (xn > x_norm) x_norm = xn;
        }
        if (x_norm < EPS) x_norm = 1.0;

        memcpy(x->data, x_new->data, n * sizeof(double));
        if (diff_norm < tol * x_norm) break;
    }

    vec_free(x_new);
    return x;
}

/* -----------------------------------------------------------------------------
 * L8: Residual Norm History — tracks convergence of each iterative method.
 * Stores residual norms at each iteration for convergence analysis.
 * The out_iter parameter returns the actual number of iterations used.
 * Returns vector of residual norms (one per iteration).
 *
 * Useful for analyzing convergence rates: asymptotic rate = (r_k/r_0)^{1/k}.
 * For CG, the rate is bounded by 2((√κ-1)/(√κ+1)).
 * --------------------------------------------------------------------------- */
Vector* iterative_residual_history(const Matrix* A, const Vector* b,
                                    const char* method_name,
                                    double tol, int max_iter,
                                    int* out_iter) {
    int n = A->rows;
    Vector* history = vec_create(max_iter + 1);
    Vector* x = vec_create(n);
    Vector* r = vec_clone(b);

    double b_norm = vec_norm(b);
    if (b_norm < EPS) b_norm = 1.0;

    int iter;
    for (iter = 0; iter < max_iter; iter++) {
        double r_norm = vec_norm(r);
        history->data[iter] = r_norm / b_norm;
        if (r_norm / b_norm < tol) break;

        if (strcmp(method_name, "jacobi") == 0) {
            for (int i = 0; i < n; i++) {
                double sum = b->data[i];
                for (int j = 0; j < n; j++)
                    if (j != i) sum -= A->data[i * n + j] * x->data[j];
                x->data[i] = sum / A->data[i * n + i];
            }
        } else {
            /* default: Richardson with ω=0.5 */
            for (int i = 0; i < n; i++)
                x->data[i] += 0.5 * r->data[i];
        }

        /* update residual */
        Vector* Ax = mat_vec_mul(A, x);
        for (int i = 0; i < n; i++)
            r->data[i] = b->data[i] - Ax->data[i];
        vec_free(Ax);
    }
    history->data[iter] = vec_norm(r) / b_norm;
    *out_iter = iter;

    /* trim history to actual iterations */
    Vector* trimmed = vec_create(iter + 1);
    memcpy(trimmed->data, history->data, (iter + 1) * sizeof(double));
    vec_free(history);
    vec_free(x);
    vec_free(r);
    return trimmed;
}
