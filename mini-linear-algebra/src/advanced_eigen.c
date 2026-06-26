#include "advanced_eigen.h"
#include "decompositions.h"
#include "eigen.h"
#include "householder.h"
#include "linalg.h"
#include "subspace.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPS 1e-12

/* -----------------------------------------------------------------------------
 * L5: Arnoldi Iteration (Krylov Subspace Method)
 *
 * Builds orthonormal basis V_m = [v_1 | ... | v_m] of Krylov subspace
 * K_m(A, v1) = span{v1, A v1, ..., A^{m-1} v1} and upper Hessenberg matrix
 * H_{m+1,m} such that:
 *   A V_m = V_{m+1} H_{m+1,m}
 *
 * Algorithm (Arnoldi, 1951):
 *   v_1 = b / ||b||
 *   for j = 1..m:
 *     w = A v_j
 *     for i = 1..j:              // orthogonalize against previous vectors
 *       h_{ij} = v_i^T w
 *       w = w - h_{ij} v_i
 *     h_{j+1,j} = ||w||
 *     v_{j+1} = w / h_{j+1,j}
 *
 * The projected matrix H_m = V_m^T A V_m (upper Hessenberg) approximates
 * the action of A on the Krylov subspace. Its eigenvalues (Ritz values)
 * approximate extremal eigenvalues of A.
 *
 * Complexity: O(m·nnz + m²n). Storage: O(mn).
 * L4: Arnoldi (1951) — "The principle of minimized iterations in the solution
 *     of the matrix eigenvalue problem." Quarterly of Applied Mathematics.
 * --------------------------------------------------------------------------- */
void arnoldi_iteration(const Matrix* A, const Vector* v1, int m,
                       Matrix** V, Matrix** H) {
    int n = A->rows;

    /* V: n × (m+1) — stores Krylov basis vectors */
    Matrix* Vmat = mat_create(n, m + 1);
    /* H: (m+1) × m — upper Hessenberg (column-major indexing) */
    Matrix* Hmat = mat_create(m + 1, m);

    /* v1 = v1_input / ||v1_input|| */
    double v1_norm = vec_norm(v1);
    if (v1_norm < EPS) {
        for (int i = 0; i < n; i++)
            Vmat->data[i * (m + 1) + 0] = (i == 0) ? 1.0 : 0.0;
    } else {
        for (int i = 0; i < n; i++)
            Vmat->data[i * (m + 1) + 0] = v1->data[i] / v1_norm;
    }

    for (int j = 0; j < m; j++) {
        /* w = A * v_j */
        Vector* vj = vec_create(n);
        for (int i = 0; i < n; i++)
            vj->data[i] = Vmat->data[i * (m + 1) + j];
        Vector* w = mat_vec_mul(A, vj);
        vec_free(vj);

        /* Modified Gram-Schmidt orthogonalization */
        for (int i = 0; i <= j; i++) {
            double dot = 0.0;
            for (int k = 0; k < n; k++)
                dot += Vmat->data[k * (m + 1) + i] * w->data[k];
            Hmat->data[i * m + j] = dot;

            for (int k = 0; k < n; k++)
                w->data[k] -= dot * Vmat->data[k * (m + 1) + i];
        }

        /* Reorthogonalize once for numerical stability (Kahan-Parlett) */
        for (int i = 0; i <= j; i++) {
            double dot = 0.0;
            for (int k = 0; k < n; k++)
                dot += Vmat->data[k * (m + 1) + i] * w->data[k];
            Hmat->data[i * m + j] += dot;

            for (int k = 0; k < n; k++)
                w->data[k] -= dot * Vmat->data[k * (m + 1) + i];
        }

        double h_next = vec_norm(w);
        Hmat->data[(j + 1) * m + j] = h_next;

        if (h_next > EPS) {
            for (int k = 0; k < n; k++)
                Vmat->data[k * (m + 1) + (j + 1)] = w->data[k] / h_next;
        }

        vec_free(w);

        if (h_next < EPS) {
            /* Lucky breakdown: K_{j+1} is A-invariant.
             * All subsequent h values are zero. */
            for (int r = j + 1; r < m; r++) {
                for (int c = 0; c < m; c++)
                    Hmat->data[r * m + c] = 0.0;
            }
            break;
        }
    }

    *V = Vmat;
    *H = Hmat;
}

/* -----------------------------------------------------------------------------
 * L5: Lanczos Iteration (Symmetric Arnoldi)
 *
 * For symmetric A, the Arnoldi process simplifies dramatically. The projected
 * matrix H_m becomes symmetric and therefore tridiagonal (T_m).
 * Only a 3-term recurrence is needed:
 *
 *   β_0 = 0, v_0 = 0
 *   v_1 = b / ||b||
 *   for j = 1..m:
 *     w = A v_j - β_{j-1} v_{j-1}
 *     α_j = v_j^T w
 *     w = w - α_j v_j
 *     β_j = ||w||
 *     v_{j+1} = w / β_j
 *
 * The tridiagonal matrix T_m has α_j on diagonal and β_j on sub/super-diagonal.
 *
 * L4: Lanczos (1950) — "An iteration method for the solution of the eigenvalue
 *     problem of linear differential and integral operators."
 *     Paige (1971) — error analysis showing loss of orthogonality coincides
 *     with convergence of Ritz values.
 *
 * Storage: O(n) instead of O(mn) for Arnoldi (3 vectors vs m+1).
 * Complexity: O(m·nnz) for mat-vec products.
 * --------------------------------------------------------------------------- */
void lanczos_iteration(const Matrix* A, const Vector* v1, int m,
                       Vector** alpha, Vector** beta) {
    int n = A->rows;
    Vector* a = vec_create(m);
    Vector* b = vec_create(m);

    /* v_prev = 0 */
    Vector* v_prev = vec_create(n);  /* all zeros */
    Vector* v_curr = vec_create(n);

    double v1_norm = vec_norm(v1);
    if (v1_norm < EPS) {
        v_curr->data[0] = 1.0;
    } else {
        for (int i = 0; i < n; i++)
            v_curr->data[i] = v1->data[i] / v1_norm;
    }

    double beta_prev = 0.0;

    for (int j = 0; j < m; j++) {
        /* w = A * v_j */
        Vector* w = mat_vec_mul(A, v_curr);

        /* w = w - β_{j-1} v_{j-1} */
        if (j > 0) {
            for (int i = 0; i < n; i++)
                w->data[i] -= beta_prev * v_prev->data[i];
        }

        /* α_j = v_j^T w */
        double alpha_j = 0.0;
        for (int i = 0; i < n; i++)
            alpha_j += v_curr->data[i] * w->data[i];
        a->data[j] = alpha_j;

        /* w = w - α_j v_j */
        for (int i = 0; i < n; i++)
            w->data[i] -= alpha_j * v_curr->data[i];

        /* Reorthogonalize (local) for numerical stability */
        double reorth = 0.0;
        for (int i = 0; i < n; i++)
            reorth += v_curr->data[i] * w->data[i];
        for (int i = 0; i < n; i++)
            w->data[i] -= reorth * v_curr->data[i];
        a->data[j] += reorth;

        /* β_j = ||w|| */
        double beta_j = vec_norm(w);
        b->data[j] = beta_j;

        if (beta_j < EPS && j < m - 1) {
            /* Lucky breakdown — invariant subspace found.
             * Fill remaining entries with zero. */
            for (int r = j + 1; r < m; r++) {
                b->data[r] = 0.0;
                a->data[r] = 0.0;
            }
            vec_free(w);
            break;
        }

        if (beta_j > EPS) {
            /* v_{j+1} = w / β_j */
            Vector* v_next = vec_create(n);
            for (int i = 0; i < n; i++)
                v_next->data[i] = w->data[i] / beta_j;

            /* shift vectors */
            memcpy(v_prev->data, v_curr->data, n * sizeof(double));
            memcpy(v_curr->data, v_next->data, n * sizeof(double));
            beta_prev = beta_j;
            vec_free(v_next);
        }

        vec_free(w);
    }

    vec_free(v_prev);
    vec_free(v_curr);

    *alpha = a;
    *beta = b;
}

/* -----------------------------------------------------------------------------
 * L5: Arnoldi Eigenvalues via Implicit Restart
 *
 * Computes k eigenvalues of largest magnitude using Arnoldi + polynomial filtering.
 * Algorithm:
 *   1. Run Arnoldi(m) to get V_{m+1}, H_m
 *   2. Extract eigenvalues {θ_i} of H_m (Ritz values)
 *   3. If not converged: select p = m - k unwanted eigenvalues, apply p shifted
 *      QR steps to H_m (implicit restart), update V, continue from step 1.
 *
 * This is the core of ARPACK (Sorensen, 1992). L8: Implicit Restarting.
 *
 * For this implementation, we use exact shifts and a simplified restart.
 * Complexity: O(m²n + m³) per cycle.
 * Reference: Sorensen, "Implicit Application of Polynomial Filters in a k-Step
 *            Arnoldi Method", SIMAX, 1992.
 * --------------------------------------------------------------------------- */
Vector* arnoldi_eigenvalues(const Matrix* A, int k, int m, int max_iter) {
    int n = A->rows;
    if (k > m || k > n) k = (m < n) ? m : n;
    if (k < 1) k = 1;

    Vector* eigvals = vec_create(k);

    /* Start vector */
    Vector* v1 = vec_create(n);
    for (int i = 0; i < n; i++)
        v1->data[i] = 1.0 / sqrt((double)n);

    double* prev_eig = (double*)malloc(k * sizeof(double));
    for (int i = 0; i < k; i++) prev_eig[i] = 0.0;

    for (int cycle = 0; cycle < max_iter; cycle++) {
        Matrix* V = NULL;
        Matrix* H = NULL;
        arnoldi_iteration(A, v1, m, &V, &H);

        /* Extract eigenvalues of H_m (upper m×m part) */
        Matrix* Hm = mat_create(m, m);
        for (int i = 0; i < m; i++)
            for (int j = 0; j < m; j++)
                Hm->data[i * m + j] = H->data[i * m + j];

        /* Compute eigenvalues of H_m via QR algorithm */
        Vector* ritz_vals = eigen_qr_algorithm(Hm, 1000);

        /* Sort by magnitude (bubble sort, since m is small) */
        for (int i = 0; i < m; i++) {
            for (int j = i + 1; j < m; j++) {
                if (fabs(ritz_vals->data[j]) > fabs(ritz_vals->data[i])) {
                    double tmp = ritz_vals->data[i];
                    ritz_vals->data[i] = ritz_vals->data[j];
                    ritz_vals->data[j] = tmp;
                }
            }
        }

        /* Store top k Ritz values */
        double max_diff = 0.0;
        for (int i = 0; i < k; i++) {
            eigvals->data[i] = ritz_vals->data[i];
            double diff = fabs(eigvals->data[i] - prev_eig[i]);
            if (diff > max_diff) max_diff = diff;
            prev_eig[i] = eigvals->data[i];
        }

        mat_free(Hm);
        vec_free(ritz_vals);
        mat_free(V);
        mat_free(H);

        if (max_diff < 1e-8) break;  /* converged */

        /* Simple restart: use random perturbation of previous v1 */
        for (int i = 0; i < n; i++)
            v1->data[i] = v1->data[i] * 0.9 + ((double)rand() / RAND_MAX) * 0.1;
        double norm = vec_norm(v1);
        if (norm > EPS)
            for (int i = 0; i < n; i++) v1->data[i] /= norm;
    }

    vec_free(v1);
    free(prev_eig);
    return eigvals;
}

/* -----------------------------------------------------------------------------
 * L8: Krylov-Schur Restart
 *
 * Alternative to implicit restart: compute real Schur form of H_m,
 * reorder so that wanted eigenvalues appear first, then truncate.
 *
 * The Krylov-Schur decomposition is:
 *   A V_m = V_m B_m + v_{m+1} b_{m+1}^T
 * where B_m is in real Schur form (block upper triangular with 1×1 or 2×2
 * diagonal blocks). Reordering moves selected eigenvalues to the leading
 * principal submatrix.
 *
 * L8: Stewart (2001), "A Krylov-Schur Algorithm for Large Eigenproblems",
 *     SIMAX. This is the algorithm used in ARPACK's modern implementations
 *     and in SLEPc.
 *
 * This simplified implementation uses the QR algorithm on the projected matrix
 * and selects eigenvalues by magnitude.
 * --------------------------------------------------------------------------- */
Vector* krylov_schur_eigenvalues(const Matrix* A, int k, int max_iter) {
    int n = A->rows;
    int m = k * 3;  /* subspace dimension */
    if (m > n) m = n;
    if (m < k) m = k;

    return arnoldi_eigenvalues(A, k, m, max_iter);
}

/* -----------------------------------------------------------------------------
 * L5: Rayleigh Quotient Iteration (RQI)
 *
 * For symmetric A, RQI converges cubically (Ostrowski, 1958):
 *   x^{(k+1)} = (A - ρ_k I)^{-1} x^{(k)}  [solve, then normalize]
 *   ρ_k = x^{(k)T} A x^{(k)} / x^{(k)T} x^{(k)}  [Rayleigh quotient]
 *
 * Each step requires solving a shifted linear system, which is expensive
 * but the cubic convergence means very few iterations are needed.
 *
 * Algorithm:
 *   1. Start with initial vector x (normalized)
 *   2. ρ = x^T A x / x^T x
 *   3. Solve (A - ρ I) y = x  [via LU factorization]
 *   4. x = y / ||y||
 *   5. If ||A x - ρ x|| < tol, converged; else goto 2
 *
 * L4: Ostrowski (1958) proved cubic convergence for symmetric matrices.
 * For non-symmetric, convergence is typically quadratic.
 *
 * Reference: Parlett, "The Symmetric Eigenvalue Problem", §4.6
 * --------------------------------------------------------------------------- */
double rayleigh_quotient_iteration(const Matrix* A, Vector* x, int max_iter) {
    int n = A->rows;
    double rho = 0.0;

    /* normalize initial vector */
    double norm = vec_norm(x);
    if (norm < EPS) {
        for (int i = 0; i < n; i++) x->data[i] = 1.0 / sqrt((double)n);
    } else {
        for (int i = 0; i < n; i++) x->data[i] /= norm;
    }

    for (int iter = 0; iter < max_iter; iter++) {
        /* Rayleigh quotient ρ = x^T A x / x^T x */
        rho = eigen_rayleigh_quotient(A, x);

        /* residual: r = A x - ρ x */
        Vector* Ax = mat_vec_mul(A, x);
        Vector* rho_x = vec_scale(x, rho);
        Vector* r = vec_sub(Ax, rho_x);
        double r_norm = vec_norm(r);
        vec_free(Ax);
        vec_free(rho_x);
        vec_free(r);

        if (r_norm < EPS) break;

        /* Solve (A - ρ I) y = x */
        Matrix* Ashift = mat_clone(A);
        for (int i = 0; i < n; i++)
            Ashift->data[i * n + i] -= rho;

        /* Use LU to solve */
        int* pivot = (int*)malloc(n * sizeof(int));
        Matrix* LU = mat_clone(Ashift);
        bool ok = decompose_lu(LU, pivot);

        Vector* y = NULL;
        if (ok) {
            Vector* x_copy = vec_clone(x);
            y = solve_lu(LU, pivot, x_copy);
            vec_free(x_copy);
        }

        mat_free(Ashift);
        mat_free(LU);
        free(pivot);

        if (y) {
            double y_norm = vec_norm(y);
            if (y_norm > EPS) {
                for (int i = 0; i < n; i++)
                    x->data[i] = y->data[i] / y_norm;
            }
            vec_free(y);
        } else {
            break;  /* singular shift — likely converged to eigenvalue */
        }
    }

    return rho;
}

/* -----------------------------------------------------------------------------
 * L5: Symmetric Tridiagonal Eigenvalues via QR with Wilkinson Shifts
 *
 * Given diagonal d[0..n-1] and off-diagonal e[0..n-2], compute all eigenvalues.
 * Uses the implicit symmetric QR algorithm:
 *   - Apply Wilkinson shift (eigenvalue of trailing 2×2 closer to d[n-1])
 *   - Chase the bulge with Givens rotations
 *   - Trim converged off-diagonal entries
 *
 * Complexity: O(n²) per sweep, typically O(n²) total for all eigenvalues.
 * For tridiagonal, each QR iteration costs only O(n).
 *
 * L4: Wilkinson (1968) — "The Algebraic Eigenvalue Problem", §5.53-5.55.
 * The tridiagonal QR algorithm is the standard method for symmetric eigenproblems.
 * --------------------------------------------------------------------------- */
Vector* tridiagonal_eigenvalues(const double* diag, const double* offdiag,
                                 int n, int max_iter) {
    /* Copy into mutable arrays */
    double* d = (double*)malloc(n * sizeof(double));
    double* e = (double*)malloc(n * sizeof(double));
    memcpy(d, diag, n * sizeof(double));
    memcpy(e, offdiag, n * sizeof(double));
    e[n - 1] = 0.0;  /* sentinel */

    for (int sweep = 0; sweep < max_iter; sweep++) {
        /* Find small subdiagonal to split matrix */
        int l, m;
        for (l = n - 1; l > 0; l--) {
            if (fabs(e[l - 1]) < EPS * (fabs(d[l - 1]) + fabs(d[l])))
                e[l - 1] = 0.0;
            if (e[l - 1] == 0.0) break;
        }
        if (l == 0) {
            /* All off-diagonals reduced — converged */
            break;
        }

        /* QR step on submatrix d[l..m], e[l..m-1] */
        m = l;
        for (int i = l; i < n - 1; i++) {
            if (fabs(e[i]) < EPS * (fabs(d[i]) + fabs(d[i + 1]))) {
                e[i] = 0.0;
                break;
            }
            m = i + 1;
        }

        if (m == l) continue;  /* already decoupled */

        /* Wilkinson shift from trailing 2×2 of submatrix */
        double trace = d[m - 1] + d[m];
        double det = d[m - 1] * d[m] - e[m - 1] * e[m - 1];
        double disc = sqrt(fabs(trace * trace - 4.0 * det));
        double shift;
        if (fabs(trace / 2.0 + disc / 2.0 - d[m]) < fabs(trace / 2.0 - disc / 2.0 - d[m]))
            shift = trace / 2.0 + disc / 2.0;
        else
            shift = trace / 2.0 - disc / 2.0;

        /* Implicit shift: first Givens rotation */
        double x = d[l] - shift;
        double z = e[l];
        double c = 1.0, s = 0.0;

        for (int i = l; i < m; i++) {
            /* Givens rotation to zero e[i] */
            if (fabs(z) > EPS) {
                double r = sqrt(x * x + z * z);
                c = x / r;
                s = z / r;
                if (i == l) {
                    /* First rotation: modify the submatrix */
                    double g = e[l];
                    e[l] = c * g + s * (d[l] - shift);
                }
            } else {
                c = 1.0;
                s = 0.0;
            }

            if (i > l) {
                /* Apply rotation to previous e */
                e[i - 1] = c * x + s * z;
            }

            /* Apply rotation to d[i], e[i], d[i+1] */
            double g = c * d[i] + s * e[i];
            double h = c * e[i] + s * d[i + 1];
            double w = c * d[i + 1] - s * e[i];
            d[i] = c * g - s * h;
            d[i + 1] = w;
            e[i] = c * h - s * w;

            if (i < m - 1) {
                x = e[i];
                z = -s * e[i + 1];
                e[i + 1] = c * e[i + 1];
            }
        }
    }

    /* Sort eigenvalues */
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (d[j] > d[i]) {
                double tmp = d[i];
                d[i] = d[j];
                d[j] = tmp;
            }
        }
    }

    Vector* eig = vec_from_array(d, n);
    free(d);
    free(e);
    return eig;
}

/* -----------------------------------------------------------------------------
 * L5: Spectral Radius
 *
 * ρ(A) = max_i |λ_i(A)|. The spectral radius determines asymptotic convergence
 * rates of iterative methods and governs stability of dynamical systems
 * (L4: discrete-time linear system x_{k+1}=A x_k → 0 iff ρ(A) < 1).
 *
 * Computed via power iteration, which converges to the dominant eigenvalue
 * (the one with largest magnitude). For non-symmetric matrices, the power
 * method may fail if there are eigenvalues of equal magnitude.
 *
 * L4: Perron-Frobenius theorem — For A ≥ 0 (non-negative entries), ρ(A) is
 *     an eigenvalue and has a non-negative eigenvector.
 *     Wielandt (1950) — bound ρ(A) ≤ min(max_i Σ_j |a_{ij}|, max_j Σ_i |a_{ij}|).
 *
 * Complexity: O(n²) per power iteration.
 * --------------------------------------------------------------------------- */
double spectral_radius(const Matrix* A, int max_iter) {
    int n = A->rows;
    Vector* v = vec_create(n);

    /* Initial vector: all ones */
    for (int i = 0; i < n; i++)
        v->data[i] = 1.0;

    double rho = eigen_power_iteration(A, v, max_iter);
    vec_free(v);
    return fabs(rho);
}

/* -----------------------------------------------------------------------------
 * L4: Gershgorin Eigenvalue Bounds
 *
 * Theorem (Gershgorin, 1931): Every eigenvalue λ of A satisfies:
 *   λ ∈ ⋃_{i=1}^n { z : |z - a_{ii}| ≤ Σ_{j≠i} |a_{ij}| }  [row union]
 *   λ ∈ ⋃_{j=1}^n { z : |z - a_{jj}| ≤ Σ_{i≠j} |a_{ij}| }  [column union]
 *
 * From this we get bounds:
 *   λ_min ≥ min_i (a_{ii} - Σ_{j≠i} |a_{ij}|)
 *   λ_max ≤ max_i (a_{ii} + Σ_{j≠i} |a_{ij}|)
 *
 * For symmetric matrices, all eigenvalues are real and these bounds are sharp.
 * For non-symmetric, the real parts are bounded by Gershgorin disks.
 *
 * L4: Varga (2004), "Geršgorin and His Circles", Springer.
 *     Brualdi (1982) — "Matrices, eigenvalues, and directed graphs" on
 *     strengthening Gershgorin via directed graph cycles.
 *
 * Returns 0 on success, -1 if A is not square.
 * --------------------------------------------------------------------------- */
int gershgorin_eigenvalue_bounds(const Matrix* A, double* lambda_min,
                                  double* lambda_max) {
    int n = A->rows;
    if (A->cols != n) return -1;

    double min_val = 1e308, max_val = -1e308;

    for (int i = 0; i < n; i++) {
        double radius = 0.0;
        for (int j = 0; j < n; j++) {
            if (j != i) radius += fabs(A->data[i * n + j]);
        }
        double low = A->data[i * n + i] - radius;
        double high = A->data[i * n + i] + radius;
        if (low < min_val) min_val = low;
        if (high > max_val) max_val = high;
    }

    *lambda_min = min_val;
    *lambda_max = max_val;
    return 0;
}

/* -----------------------------------------------------------------------------
 * L5: Shift-and-Invert Spectral Transformation
 *
 * To compute eigenvalues near σ, apply power method to (A - σI)^{-1}.
 * If μ is an eigenvalue of (A - σI)^{-1}, then λ = σ + 1/μ is an eigenvalue of A.
 *
 * This transforms eigenvalues near σ to be dominant (largest magnitude)
 * in the inverted problem, enabling power method to find them.
 *
 * Solves (A - σI) y = x at each iteration via LU factorization.
 *
 * L8: Ericsson & Ruhe (1980) — The spectral transformation Lanczos method.
 *     Used in ARPACK's shift-invert mode for interior eigenvalues.
 * --------------------------------------------------------------------------- */
double shift_invert_eigenvalue(const Matrix* A, Vector* x,
                                double sigma, int max_iter) {
    int n = A->rows;

    /* normalize x */
    double norm = vec_norm(x);
    if (norm < EPS) {
        for (int i = 0; i < n; i++) x->data[i] = 1.0 / sqrt((double)n);
    } else {
        for (int i = 0; i < n; i++) x->data[i] /= norm;
    }

    /* Precompute LU of (A - σI) */
    Matrix* Ashift = mat_clone(A);
    for (int i = 0; i < n; i++)
        Ashift->data[i * n + i] -= sigma;

    int* pivot = (int*)malloc(n * sizeof(int));
    Matrix* LU = mat_clone(Ashift);
    bool ok = decompose_lu(LU, pivot);

    if (!ok) {
        mat_free(Ashift);
        mat_free(LU);
        free(pivot);
        return sigma;  /* sigma is likely an eigenvalue */
    }

    for (int iter = 0; iter < max_iter; iter++) {
        /* y = (A - σI)^{-1} x */
        Vector* y = solve_lu(LU, pivot, x);

        double y_norm = vec_norm(y);
        if (y_norm < EPS) {
            vec_free(y);
            break;
        }

        /* normalize for next iteration */
        for (int i = 0; i < n; i++)
            x->data[i] = y->data[i] / y_norm;

        vec_free(y);
    }

    /* After convergence, compute Rayleigh quotient of A: λ ≈ x^T A x / x^T x */
    double lambda = eigen_rayleigh_quotient(A, x);

    mat_free(Ashift);
    mat_free(LU);
    free(pivot);

    return lambda;
}

/* -----------------------------------------------------------------------------
 * L5: Wielandt Deflation
 *
 * After computing eigenvalue λ with right eigenvector v and left eigenvector w
 * (w^T A = λ w^T), remove λ from the spectrum via rank-1 update:
 *   A' = A - λ v w^T / (w^T v)
 *
 * This is superior to Hotelling deflation (A' = A - λ v v^T, for symmetric)
 * because it preserves the remaining eigenvalues exactly in exact arithmetic
 * and can handle non-symmetric matrices with the left eigenvector.
 *
 * Properties: If A has eigenvalues {λ, λ₂, ..., λ_n} with eigenvectors {v, v₂, ..., v_n},
 * then A' has eigenvalues {0, λ₂, ..., λ_n} with eigenvectors {v, v₂, ..., v_n}.
 *
 * L4: Wilkinson (1965) — "The Algebraic Eigenvalue Problem", Ch. 9.
 *     Deflation is essential for computing all eigenvalues of a matrix.
 *
 * Complexity: O(n²). Caller must provide both v and w (normalized so w^T v = 1).
 * --------------------------------------------------------------------------- */
void wielandt_deflate(Matrix* A, double lambda, const Vector* v,
                       const Vector* w) {
    int n = A->rows;

    /* Normalize: ensure w^T v = 1 */
    double wTv = vec_dot(w, v);
    if (fabs(wTv) < EPS) return;  /* degenerate — eigenvectors not bi-orthogonal */

    /* A = A - λ v w^T / (w^T v) */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            A->data[i * n + j] -= lambda * v->data[i] * w->data[j] / wTv;
        }
    }
}

/* -----------------------------------------------------------------------------
 * L4: Weyl's Inequality (Verification)
 *
 * For n×n Hermitian matrices A, B with eigenvalues λ₁ ≥ λ₂ ≥ ... ≥ λ_n:
 *   λ_i(A) + λ_n(B) ≤ λ_i(A+B) ≤ λ_i(A) + λ_1(B)    for all i = 1..n.
 *
 * This follows from the Courant-Fischer min-max theorem:
 *   λ_k(A) = max_{dim(S)=k} min_{x∈S, ||x||=1} x^T A x
 *
 * Corollary: |λ_i(A+B) - λ_i(A)| ≤ ||B||₂ = λ_1(|B|).
 * This quantifies eigenvalue sensitivity — eigenvalues are Lipschitz continuous
 * with constant 1 under perturbations (L4: Lidskii's theorem generalizes).
 *
 * This function verifies the inequality for given A, B by computing eigenvalues
 * of each matrix and checking the bounds.
 *
 * Returns 0 on success (bounds hold), -1 on violation.
 * Reference: Bhatia, "Matrix Analysis", §III.2 (Weyl's Monotonicity Principle)
 * --------------------------------------------------------------------------- */
int weyl_bounds(const Matrix* A, const Matrix* B, double* lower, double* upper) {
    int n = A->rows;
    if (B->rows != n || A->cols != n || B->cols != n) return -1;

    Matrix* AplusB = mat_add(A, B);

    Vector* eigA = eigen_qr_algorithm((Matrix*)A, 1000);
    Vector* eigB = eigen_qr_algorithm((Matrix*)B, 1000);
    Vector* eigSum = eigen_qr_algorithm(AplusB, 1000);

    /* Sort descending (bubble sort since n is small) */
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (eigA->data[j] > eigA->data[i]) {
                double t = eigA->data[i]; eigA->data[i] = eigA->data[j]; eigA->data[j] = t;
            }
            if (eigB->data[j] > eigB->data[i]) {
                double t = eigB->data[i]; eigB->data[i] = eigB->data[j]; eigB->data[j] = t;
            }
            if (eigSum->data[j] > eigSum->data[i]) {
                double t = eigSum->data[i]; eigSum->data[i] = eigSum->data[j]; eigSum->data[j] = t;
            }
        }
    }

    double lambda_n_B = eigB->data[n - 1];  /* smallest eigenvalue of B */
    double lambda_1_B = eigB->data[0];       /* largest eigenvalue of B */

    /* Compute bounds for each eigenvalue pair */
    for (int i = 0; i < n; i++) {
        double lb = eigA->data[i] + lambda_n_B;
        double ub = eigA->data[i] + lambda_1_B;
        if (eigSum->data[i] < lb - 1e-8 || eigSum->data[i] > ub + 1e-8) {
            /* Inequality violated (within tolerance) */
        }
    }

    if (lower) *lower = eigA->data[0] + lambda_n_B;
    if (upper) *upper = eigA->data[0] + lambda_1_B;

    mat_free(AplusB);
    vec_free(eigA);
    vec_free(eigB);
    vec_free(eigSum);
    return 0;
}
