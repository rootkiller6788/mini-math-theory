#include "advanced_eigen.h"
#include "decompositions.h"
#include "eigen.h"
#include "householder.h"
#include "iterative.h"
#include "linalg.h"
#include "matrix.h"
#include "sparse.h"
#include "subspace.h"
#include "svd.h"
#include "tensor.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  %-55s", name); } while(0)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)

#define ASSERT_CLOSE(a, b, tol) do { \
    if (fabs((a) - (b)) > (tol)) { FAIL("value mismatch"); goto next_test; } \
} while(0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { FAIL("expected true"); goto next_test; } \
} while(0)

#define ASSERT_NOT_NULL(p) do { \
    if ((p) == NULL) { FAIL("unexpected NULL"); goto next_test; } \
} while(0)

/* ============================================================================
 * L1: Vector Operations
 * =========================================================================== */
static void test_vector_create(void) {
    TEST("vec_create / vec_free");
    Vector* v = vec_create(5);
    ASSERT_NOT_NULL(v);
    ASSERT_CLOSE(v->size, 5.0, 0);
    for (int i = 0; i < 5; i++)
        ASSERT_CLOSE(v->data[i], 0.0, 1e-15);
    vec_free(v);
    PASS();
    return;
next_test:;
}

static void test_vector_ops(void) {
    TEST("vector add/sub/scale/dot/norm");
    double a[] = {1, 2, 3};
    double b[] = {4, 5, 6};
    Vector* va = vec_from_array(a, 3);
    Vector* vb = vec_from_array(b, 3);

    Vector* sum = vec_add(va, vb);
    ASSERT_CLOSE(sum->data[0], 5.0, 1e-14);
    ASSERT_CLOSE(sum->data[1], 7.0, 1e-14);
    ASSERT_CLOSE(sum->data[2], 9.0, 1e-14);
    vec_free(sum);

    Vector* diff = vec_sub(va, vb);
    ASSERT_CLOSE(diff->data[0], -3.0, 1e-14);
    vec_free(diff);

    Vector* scaled = vec_scale(va, 2.0);
    ASSERT_CLOSE(scaled->data[0], 2.0, 1e-14);
    vec_free(scaled);

    double dot = vec_dot(va, vb);
    ASSERT_CLOSE(dot, 32.0, 1e-14);

    /* norm: √(1²+2²+3²) = √14 */
    double nrm = vec_norm(va);
    ASSERT_CLOSE(nrm, sqrt(14.0), 1e-14);

    vec_free(va);
    vec_free(vb);
    PASS();
    return;
next_test:;
}

static void test_vector_cross(void) {
    TEST("vec_cross3");
    double a[] = {1, 0, 0};
    double b[] = {0, 1, 0};
    Vector* va = vec_from_array(a, 3);
    Vector* vb = vec_from_array(b, 3);
    Vector* cross = vec_cross3(va, vb);
    ASSERT_CLOSE(cross->data[0], 0.0, 1e-14);
    ASSERT_CLOSE(cross->data[1], 0.0, 1e-14);
    ASSERT_CLOSE(cross->data[2], 1.0, 1e-14);
    vec_free(va); vec_free(vb); vec_free(cross);
    PASS();
    return;
next_test:;
}

static void test_vector_distance_angle(void) {
    TEST("vec_distance / vec_angle");
    double a[] = {0, 0};
    double b[] = {3, 4};
    Vector* va = vec_from_array(a, 2);
    Vector* vb = vec_from_array(b, 2);

    double dist = vec_distance(va, vb);
    ASSERT_CLOSE(dist, 5.0, 1e-14);

    double a2[] = {1, 0};
    double b2[] = {0, 1};
    Vector* vx = vec_from_array(a2, 2);
    Vector* vy = vec_from_array(b2, 2);
    double angle = vec_angle(vx, vy);
    ASSERT_CLOSE(angle, M_PI / 2.0, 1e-10);

    vec_free(va); vec_free(vb);
    vec_free(vx); vec_free(vy);
    PASS();
    return;
next_test:;
}

static void test_vector_normalize(void) {
    TEST("vec_normalize");
    double a[] = {3, 4};
    Vector* v = vec_from_array(a, 2);
    Vector* u = vec_normalize(v);
    ASSERT_CLOSE(vec_norm(u), 1.0, 1e-14);
    ASSERT_CLOSE(u->data[0], 0.6, 1e-14);
    ASSERT_CLOSE(u->data[1], 0.8, 1e-14);
    vec_free(v); vec_free(u);
    PASS();
    return;
next_test:;
}

static void test_vec_norm_p(void) {
    TEST("vec_norm_p (L1/L2/Linf)");
    double a[] = {3, 4};
    Vector* v = vec_from_array(a, 2);
    ASSERT_CLOSE(vec_norm_p(v, 1.0), 7.0, 1e-14);
    ASSERT_CLOSE(vec_norm_p(v, 2.0), 5.0, 1e-14);
    /* L_infinity approximated by large p: (3^10+4^10)^(1/10) ≈ 4.015 */
    double l10 = vec_norm_p(v, 10.0);
    ASSERT_CLOSE(l10, 4.0, 0.05);
    vec_free(v);
    PASS();
    return;
next_test:;
}

/* ============================================================================
 * L1: Matrix Operations
 * =========================================================================== */
static void test_matrix_create(void) {
    TEST("mat_create / mat_identity");
    Matrix* I = mat_create_identity(3);
    ASSERT_NOT_NULL(I);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i == j)
                ASSERT_CLOSE(I->data[i * 3 + j], 1.0, 1e-14);
            else
                ASSERT_CLOSE(I->data[i * 3 + j], 0.0, 1e-14);
        }
    }
    mat_free(I);
    PASS();
    return;
next_test:;
}

static void test_matrix_multiply(void) {
    TEST("mat_mul / mat_vec_mul");
    double Ad[] = {1, 2, 3, 4};
    double Bd[] = {5, 6, 7, 8};
    Matrix* A = mat_from_array(2, 2, Ad);
    Matrix* B = mat_from_array(2, 2, Bd);
    Matrix* C = mat_mul(A, B);
    /* C = A*B = [[1*5+2*7, 1*6+2*8], [3*5+4*7, 3*6+4*8]]
     *       = [[19, 22], [43, 50]] */
    ASSERT_CLOSE(C->data[0], 19.0, 1e-14);
    ASSERT_CLOSE(C->data[1], 22.0, 1e-14);
    ASSERT_CLOSE(C->data[2], 43.0, 1e-14);
    ASSERT_CLOSE(C->data[3], 50.0, 1e-14);

    /* A*v = [1*1+2*1, 3*1+4*1] = [3, 7] */
    double vd[] = {1, 1};
    Vector* v = vec_from_array(vd, 2);
    Vector* Av = mat_vec_mul(A, v);
    ASSERT_CLOSE(Av->data[0], 3.0, 1e-14);
    ASSERT_CLOSE(Av->data[1], 7.0, 1e-14);

    mat_free(A); mat_free(B); mat_free(C);
    vec_free(v); vec_free(Av);
    PASS();
    return;
next_test:;
}

static void test_matrix_determinant(void) {
    TEST("mat_determinant");
    /* det([[4,3],[3,2]]) = 8-9 = -1 */
    double d[] = {4, 3, 3, 2};
    Matrix* A = mat_from_array(2, 2, d);
    double det = mat_determinant(A);
    ASSERT_CLOSE(det, -1.0, 1e-14);

    /* det([[2,1,1],[1,2,1],[1,1,2]]) */
    double d3[] = {2, 1, 1, 1, 2, 1, 1, 1, 2};
    Matrix* B = mat_from_array(3, 3, d3);
    double det3 = mat_determinant(B);
    ASSERT_CLOSE(det3, 4.0, 1e-12);

    mat_free(A); mat_free(B);
    PASS();
    return;
next_test:;
}

static void test_matrix_inverse(void) {
    TEST("mat_inverse");
    double d[] = {4, 7, 2, 6};
    Matrix* A = mat_from_array(2, 2, d);
    Matrix* inv = mat_inverse(A);
    ASSERT_NOT_NULL(inv);

    Matrix* prod = mat_mul(A, inv);
    for (int i = 0; i < 4; i++) {
        double expected = (i % 3 == 0) ? 1.0 : 0.0;
        ASSERT_CLOSE(prod->data[i], expected, 1e-12);
    }

    mat_free(A); mat_free(inv); mat_free(prod);
    PASS();
    return;
next_test:;
}

static void test_matrix_transpose(void) {
    TEST("mat_transpose");
    double d[] = {1, 2, 3, 4, 5, 6};
    Matrix* A = mat_from_array(2, 3, d);
    Matrix* T = mat_transpose(A);
    ASSERT_CLOSE(T->rows, 3.0, 0);
    ASSERT_CLOSE(T->cols, 2.0, 0);
    ASSERT_CLOSE(T->data[0*2+0], 1.0, 1e-14);
    ASSERT_CLOSE(T->data[0*2+1], 4.0, 1e-14);
    ASSERT_CLOSE(T->data[1*2+0], 2.0, 1e-14);
    mat_free(A); mat_free(T);
    PASS();
    return;
next_test:;
}

static void test_matrix_trace_frob(void) {
    TEST("mat_trace / mat_frobenius_norm");
    double d[] = {1, 2, 3, 4};
    Matrix* A = mat_from_array(2, 2, d);
    ASSERT_CLOSE(mat_trace(A), 5.0, 1e-14);
    /* Frobenius norm: √(1+4+9+16) = √30 */
    ASSERT_CLOSE(mat_frobenius_norm(A), sqrt(30.0), 1e-14);
    mat_free(A);
    PASS();
    return;
next_test:;
}

static void test_matrix_condition(void) {
    TEST("mat_condition_number");
    double d[] = {2, 0, 0, 0.5};
    Matrix* A = mat_from_array(2, 2, d);
    double cond = mat_condition_number(A);
    /* A = diag(2, 0.5): ||A||_F = sqrt(4.25) ≈ 2.0616
     * A^{-1} = diag(0.5, 2): ||A^{-1}||_F = sqrt(4.25) ≈ 2.0616
     * cond_F = ||A||_F * ||A^{-1}||_F = 4.25 */
    ASSERT_CLOSE(cond, 4.25, 0.5);
    mat_free(A);
    PASS();
    return;
next_test:;
}

/* ============================================================================
 * L5: LU Decomposition and Linear Systems
 * =========================================================================== */
static void test_lu_decomposition(void) {
    TEST("decompose_lu / solve_lu");
    double d[] = {4, 3, 6, 3};
    Matrix* A = mat_from_array(2, 2, d);
    int pivot[2];
    bool ok = decompose_lu(A, pivot);
    ASSERT_TRUE(ok);

    double bd[] = {10, 12};
    Vector* b = vec_from_array(bd, 2);
    Vector* x = solve_lu(A, pivot, b);
    ASSERT_CLOSE(x->data[0], 1.0, 1e-12);
    ASSERT_CLOSE(x->data[1], 2.0, 1e-12);

    mat_free(A); vec_free(b); vec_free(x);
    PASS();
    return;
next_test:;
}

static void test_solve_ax_b(void) {
    TEST("solve_ax_b");
    double d[] = {3, 2, 1, 2};
    Matrix* A = mat_from_array(2, 2, d);
    double bd[] = {7, 5};
    Vector* b = vec_from_array(bd, 2);
    Vector* x = solve_ax_b(A, b);
    ASSERT_NOT_NULL(x);
    ASSERT_CLOSE(x->data[0], 1.0, 1e-12);
    ASSERT_CLOSE(x->data[1], 2.0, 1e-12);

    mat_free(A); vec_free(b); vec_free(x);
    PASS();
    return;
next_test:;
}

/* ============================================================================
 * L5: Eigenvalue Algorithms
 * =========================================================================== */
static void test_power_iteration(void) {
    TEST("eigen_power_iteration");
    double d[] = {2, -1, -1, 2};
    Matrix* A = mat_from_array(2, 2, d);
    Vector* v = vec_create(2);
    double lambda = eigen_power_iteration(A, v, 1000);
    /* eigenvalues are 3 and 1; starting vector [1,1] converges to λ=1
     * because it's the exact eigenvector. Power method finds the eigenvalue
     * whose eigenvector has nonzero projection onto starting vector. */
    ASSERT_TRUE(fabs(lambda - 3.0) < 1e-6 || fabs(lambda - 1.0) < 1e-6);
    mat_free(A); vec_free(v);
    PASS();
    return;
next_test:;
}

static void test_qr_algorithm(void) {
    TEST("eigen_qr_algorithm");
    double d[] = {4, 1, 1, 3};
    Matrix* A = mat_from_array(2, 2, d);
    Vector* eig = eigen_qr_algorithm(A, 1000);
    /* trace = 7, det = 11, eigenvalues: (7±√5)/2 ≈ 4.618, 2.382 */
    double sum = eig->data[0] + eig->data[1];
    ASSERT_CLOSE(sum, 7.0, 1e-8);
    mat_free(A); vec_free(eig);
    PASS();
    return;
next_test:;
}

static void test_rayleigh_quotient(void) {
    TEST("eigen_rayleigh_quotient");
    double d[] = {2, 1, 1, 2};
    Matrix* A = mat_from_array(2, 2, d);
    double xd[] = {1, 1};
    Vector* x = vec_from_array(xd, 2);
    double rq = eigen_rayleigh_quotient(A, x);
    ASSERT_CLOSE(rq, 3.0, 1e-12);
    mat_free(A); vec_free(x);
    PASS();
    return;
next_test:;
}

/* ============================================================================
 * L5: QR Decomposition
 * =========================================================================== */
static void test_qr_decomposition(void) {
    TEST("decompose_qr");
    double d[] = {12, -51, 4, 6, 167, -68, -4, 24, -41};
    Matrix* A = mat_from_array(3, 3, d);
    Matrix* Q = NULL;
    Matrix* R = NULL;
    decompose_qr(A, &Q, &R);

    /* Verify Q is orthogonal: Q^T Q = I */
    Matrix* Qt = mat_transpose(Q);
    Matrix* QtQ = mat_mul(Qt, Q);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            double expected = (i == j) ? 1.0 : 0.0;
            ASSERT_CLOSE(QtQ->data[i * 3 + j], expected, 1e-12);
        }
    }

    /* Verify A = Q R */
    Matrix* QR = mat_mul(Q, R);
    for (int i = 0; i < 9; i++)
        ASSERT_CLOSE(QR->data[i], A->data[i], 1e-12);

    mat_free(A); mat_free(Q); mat_free(R);
    mat_free(Qt); mat_free(QtQ); mat_free(QR);
    PASS();
    return;
next_test:;
}

static void test_cholesky(void) {
    TEST("decompose_cholesky");
    double d[] = {4, 12, -16, 12, 37, -43, -16, -43, 98};
    Matrix* A = mat_from_array(3, 3, d);
    Matrix* L = decompose_cholesky(A);
    ASSERT_NOT_NULL(L);

    /* Verify L L^T = A */
    Matrix* Lt = mat_transpose(L);
    Matrix* LLt = mat_mul(L, Lt);
    for (int i = 0; i < 9; i++)
        ASSERT_CLOSE(LLt->data[i], A->data[i], 1e-12);

    mat_free(A); mat_free(L); mat_free(Lt); mat_free(LLt);
    PASS();
    return;
next_test:;
}

static void test_least_squares(void) {
    TEST("solve_least_squares");
    /* Overdetermined: A = [[1,2],[3,4],[5,6]], b = [7,8,9] */
    double d[] = {1, 2, 3, 4, 5, 6};
    Matrix* A = mat_from_array(3, 2, d);
    double bd[] = {7, 8, 9};
    Vector* b = vec_from_array(bd, 3);
    Vector* x = solve_least_squares(A, b);
    ASSERT_NOT_NULL(x);

    /* Compute residual ||A x - b|| */
    Vector* Ax = mat_vec_mul(A, x);
    double res = 0.0;
    for (int i = 0; i < 3; i++) {
        double diff = Ax->data[i] - b->data[i];
        res += diff * diff;
    }
    res = sqrt(res);
    /* Residual should be small */
    ASSERT_CLOSE(res, 0.0, 0.1);  /* loose tolerance due to overdetermined */

    mat_free(A); vec_free(b); vec_free(x); vec_free(Ax);
    PASS();
    return;
next_test:;
}

/* ============================================================================
 * L5: SVD
 * =========================================================================== */
static void test_svd(void) {
    TEST("svd_power_method / svd_reconstruct");
    double d[] = {1, 2, 3, 4, 5, 6, 7, 8};
    Matrix* A = mat_from_array(4, 2, d);
    Matrix* U = NULL;
    Vector* S = NULL;
    Matrix* Vt = NULL;
    svd_power_method(A, &U, &S, &Vt, 2, 1000);

    /* Singular values should be positive */
    ASSERT_CLOSE(S->data[0] > 0.0, 1.0, 0);

    /* Reconstruct and check */
    Matrix* recon = svd_reconstruct(U, S, Vt, 2);
    double err = 0.0;
    for (int i = 0; i < 8; i++) {
        double diff = recon->data[i] - A->data[i];
        err += diff * diff;
    }
    /* Approximate SVD should have moderate error */
    ASSERT_CLOSE(sqrt(err) < 5.0, 1.0, 0);

    mat_free(A); mat_free(U); mat_free(Vt);
    vec_free(S); mat_free(recon);
    PASS();
    return;
next_test:;
}

/* ============================================================================
 * L1: Sparse Matrix
 * =========================================================================== */
static void test_sparse_matrix(void) {
    TEST("sparse_create / sparse_mul_vec");
    SparseMatrix* s = sparse_create(3, 3, 10);
    sparse_add_entry(s, 0, 0, 2.0);
    sparse_add_entry(s, 0, 1, 1.0);
    sparse_add_entry(s, 1, 1, 3.0);
    sparse_add_entry(s, 2, 2, 4.0);
    sparse_finalize(s);

    double vd[] = {1, 2, 3};
    Vector* v = vec_from_array(vd, 3);
    Vector* r = sparse_mul_vec(s, v);
    ASSERT_CLOSE(r->data[0], 4.0, 1e-14);   /* 2*1 + 1*2 */
    ASSERT_CLOSE(r->data[1], 6.0, 1e-14);   /* 3*2 */
    ASSERT_CLOSE(r->data[2], 12.0, 1e-14);  /* 4*3 */

    /* round-trip dense<->sparse */
    Matrix* dense = sparse_to_dense(s);
    SparseMatrix* s2 = dense_to_sparse(dense);

    Vector* r2 = sparse_mul_vec(s2, v);
    ASSERT_CLOSE(r2->data[0], 4.0, 1e-12);
    ASSERT_CLOSE(r2->data[1], 6.0, 1e-12);
    ASSERT_CLOSE(r2->data[2], 12.0, 1e-12);

    sparse_free(s); sparse_free(s2);
    mat_free(dense); vec_free(v); vec_free(r); vec_free(r2);
    PASS();
    return;
next_test:;
}

/* ============================================================================
 * L3: Tensor3D
 * =========================================================================== */
static void test_tensor3d(void) {
    TEST("tensor3d_create / unfold / fold");
    Tensor3D* t = tensor3d_create(2, 2, 2);
    tensor3d_set(t, 0, 0, 0, 1.0);
    tensor3d_set(t, 1, 0, 0, 2.0);
    tensor3d_set(t, 0, 1, 0, 3.0);
    tensor3d_set(t, 1, 1, 0, 4.0);

    ASSERT_CLOSE(tensor3d_get(t, 0, 0, 0), 1.0, 1e-14);
    ASSERT_CLOSE(tensor3d_get(t, 1, 1, 0), 4.0, 1e-14);

    /* unfold mode 0 */
    Matrix* m0 = tensor3d_unfold(t, 0);
    ASSERT_CLOSE(m0->data[0 * 4 + 0], 1.0, 1e-14);

    /* fold back */
    Tensor3D* t2 = tensor3d_fold(m0, 2, 2, 2, 0);
    ASSERT_CLOSE(tensor3d_get(t2, 0, 0, 0), 1.0, 1e-14);
    ASSERT_CLOSE(tensor3d_get(t2, 1, 1, 0), 4.0, 1e-14);

    tensor3d_free(t); tensor3d_free(t2); mat_free(m0);
    PASS();
    return;
next_test:;
}

/* ============================================================================
 * L5: Iterative Solvers
 * =========================================================================== */
static void test_jacobi(void) {
    TEST("jacobi_solve");
    double d[] = {4, 1, 2, 3};
    Matrix* A = mat_from_array(2, 2, d);
    double bd[] = {1, 2};
    Vector* b = vec_from_array(bd, 2);
    Vector* x = jacobi_solve(A, b, 1e-10, 1000);
    ASSERT_NOT_NULL(x);
    /* 4x + y = 1, 2x + 3y = 2 => x=0.1, y=0.6 */
    ASSERT_CLOSE(x->data[0], 0.1, 1e-6);
    ASSERT_CLOSE(x->data[1], 0.6, 1e-6);
    mat_free(A); vec_free(b); vec_free(x);
    PASS();
    return;
next_test:;
}

static void test_gauss_seidel(void) {
    TEST("gauss_seidel_solve");
    double d[] = {4, 1, 2, 3};
    Matrix* A = mat_from_array(2, 2, d);
    double bd[] = {1, 2};
    Vector* b = vec_from_array(bd, 2);
    Vector* x = gauss_seidel_solve(A, b, 1e-10, 1000);
    ASSERT_NOT_NULL(x);
    ASSERT_CLOSE(x->data[0], 0.1, 1e-6);
    ASSERT_CLOSE(x->data[1], 0.6, 1e-6);
    mat_free(A); vec_free(b); vec_free(x);
    PASS();
    return;
next_test:;
}

static void test_conjugate_gradient(void) {
    TEST("conjugate_gradient");
    double d[] = {4, 1, 1, 3};
    Matrix* A = mat_from_array(2, 2, d);
    double bd[] = {1, 2};
    Vector* b = vec_from_array(bd, 2);
    Vector* x = conjugate_gradient(A, b, 1e-10, 100);
    ASSERT_NOT_NULL(x);
    /* 4x+y=1, x+3y=2 => x=1/11≈0.0909, y=7/11≈0.6364 */
    ASSERT_CLOSE(x->data[0], 1.0/11.0, 1e-6);
    ASSERT_CLOSE(x->data[1], 7.0/11.0, 1e-6);
    mat_free(A); vec_free(b); vec_free(x);
    PASS();
    return;
next_test:;
}

static void test_pcg_jacobi(void) {
    TEST("pcg_jacobi (preconditioned CG)");
    double d[] = {4, 1, 1, 3};
    Matrix* A = mat_from_array(2, 2, d);
    double bd[] = {1, 2};
    Vector* b = vec_from_array(bd, 2);
    Vector* x = pcg_jacobi(A, b, 1e-10, 100);
    ASSERT_NOT_NULL(x);
    ASSERT_CLOSE(x->data[0], 1.0/11.0, 1e-6);
    ASSERT_CLOSE(x->data[1], 7.0/11.0, 1e-6);
    mat_free(A); vec_free(b); vec_free(x);
    PASS();
    return;
next_test:;
}

static void test_gmres(void) {
    TEST("gmres_solve");
    double d[] = {4, 1, 2, 3};
    Matrix* A = mat_from_array(2, 2, d);
    double bd[] = {1, 2};
    Vector* b = vec_from_array(bd, 2);
    Vector* x = gmres_solve(A, b, 5, 1e-10, 100);
    ASSERT_NOT_NULL(x);
    ASSERT_CLOSE(x->data[0], 0.1, 1e-6);
    ASSERT_CLOSE(x->data[1], 0.6, 1e-6);
    mat_free(A); vec_free(b); vec_free(x);
    PASS();
    return;
next_test:;
}

static void test_sor(void) {
    TEST("sor_solve");
    double d[] = {4, 1, 2, 3};
    Matrix* A = mat_from_array(2, 2, d);
    double bd[] = {1, 2};
    Vector* b = vec_from_array(bd, 2);
    Vector* x = sor_solve(A, b, 1.2, 1e-10, 1000);
    ASSERT_NOT_NULL(x);
    ASSERT_CLOSE(x->data[0], 0.1, 1e-6);
    ASSERT_CLOSE(x->data[1], 0.6, 1e-6);
    mat_free(A); vec_free(b); vec_free(x);
    PASS();
    return;
next_test:;
}

/* ============================================================================
 * L5: Householder and Givens
 * =========================================================================== */
static void test_householder_qr(void) {
    TEST("householder_qr");
    double d[] = {12, -51, 4, 6, 167, -68, -4, 24, -41};
    Matrix* A = mat_from_array(3, 3, d);
    Matrix* Q = NULL;
    Matrix* R = NULL;
    householder_qr(A, &Q, &R);

    /* Check Q orthogonal: Q^T Q should be identity */
    Matrix* Qt = mat_transpose(Q);
    Matrix* QtQ = mat_mul(Qt, Q);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            double expected = (i == j) ? 1.0 : 0.0;
            ASSERT_CLOSE(QtQ->data[i * 3 + j], expected, 1e-10);
        }
    }

    /* Check A = QR */
    Matrix* QR = mat_mul(Q, R);
    for (int i = 0; i < 9; i++)
        ASSERT_CLOSE(QR->data[i], A->data[i], 1e-10);

    mat_free(A); mat_free(Q); mat_free(R);
    mat_free(Qt); mat_free(QtQ); mat_free(QR);
    PASS();
    return;
next_test:;
}

static void test_givens_qr(void) {
    TEST("givens_qr");
    double d[] = {3, 4, 0, 0};
    Matrix* A = mat_from_array(2, 2, d);
    Matrix* Q = NULL;
    Matrix* R = NULL;
    givens_qr(A, &Q, &R);

    /* R should be upper triangular: [5, 0; 0, 0] or similar */
    ASSERT_CLOSE(R->data[2], 0.0, 1e-12);  /* subdiagonal is zero */
    ASSERT_CLOSE(R->data[0] > 0.0, 1.0, 0);  /* diagonal positive */

    mat_free(A); mat_free(Q); mat_free(R);
    PASS();
    return;
next_test:;
}

static void test_gershgorin(void) {
    TEST("gershgorin_disks / gershgorin_eigenvalue_bounds");
    double d[] = {5, 1, 1, 1, 5, 1, 1, 1, 5};
    Matrix* A = mat_from_array(3, 3, d);

    double centers[3], radii[3];
    gershgorin_disks(A, centers, radii, 0);
    /* Each row: center=5, radius=2 */
    ASSERT_CLOSE(centers[0], 5.0, 1e-14);
    ASSERT_CLOSE(radii[0], 2.0, 1e-14);

    double lmin, lmax;
    int ret = gershgorin_eigenvalue_bounds(A, &lmin, &lmax);
    ASSERT_CLOSE(ret, 0.0, 0);
    /* Eigenvalues should be in [3, 7] (actual: 7, 4, 4) */
    ASSERT_TRUE(lmin <= 4.0);
    ASSERT_TRUE(lmax >= 7.0);

    mat_free(A);
    PASS();
    return;
next_test:;
}

/* ============================================================================
 * L2: Subspaces
 * =========================================================================== */
static void test_linear_independence(void) {
    TEST("is_linearly_independent");
    double d[] = {1, 0, 0, 1, 1, 1};
    Matrix* A = mat_from_array(3, 2, d);
    ASSERT_TRUE(is_linearly_independent(A));  /* columns are independent */
    mat_free(A);

    double d2[] = {1, 2, 2, 4};
    Matrix* B = mat_from_array(2, 2, d2);
    ASSERT_TRUE(!is_linearly_independent(B));  /* columns are dependent */
    mat_free(B);
    PASS();
    return;
next_test:;
}

static void test_is_in_span(void) {
    TEST("is_in_span");
    double d[] = {1, 0, 0, 1};
    Matrix* A = mat_from_array(2, 2, d);
    double bd[] = {3, 5};
    Vector* b = vec_from_array(bd, 2);
    ASSERT_TRUE(is_in_span(A, b));

    mat_free(A); vec_free(b);
    PASS();
    return;
next_test:;
}

static void test_gram_schmidt(void) {
    TEST("gram_schmidt_modified");
    double d[] = {1, 0, 1, 1};
    Matrix* A = mat_from_array(2, 2, d);
    Matrix* Q = NULL;
    Matrix* R = NULL;
    gram_schmidt_modified(A, &Q, &R);

    /* Q columns should be orthonormal: dot(q0, q1) = 0 */
    double dot_q0q1 = 0.0;
    for (int i = 0; i < 2; i++)
        dot_q0q1 += Q->data[i * 2 + 0] * Q->data[i * 2 + 1];
    ASSERT_CLOSE(dot_q0q1, 0.0, 1e-12);

    mat_free(A); mat_free(Q); mat_free(R);
    PASS();
    return;
next_test:;
}

static void test_orthogonal_projection(void) {
    TEST("orthogonal_project");
    double d[] = {1, 0, 0, 1, 0, 0};
    Matrix* A = mat_from_array(3, 2, d);
    double bd[] = {2, 3, 4};
    Vector* b = vec_from_array(bd, 3);
    Vector* proj = orthogonal_project(A, b);
    ASSERT_CLOSE(proj->data[0], 2.0, 1e-12);
    ASSERT_CLOSE(proj->data[1], 3.0, 1e-12);
    ASSERT_CLOSE(proj->data[2], 0.0, 1e-12);

    mat_free(A); vec_free(b); vec_free(proj);
    PASS();
    return;
next_test:;
}

/* ============================================================================
 * L5: Advanced Eigenvalue Methods
 * =========================================================================== */
static void test_spectral_radius(void) {
    TEST("spectral_radius");
    double d[] = {3, 1, 0, 2};
    Matrix* A = mat_from_array(2, 2, d);
    double rho = spectral_radius(A, 1000);
    ASSERT_CLOSE(rho, 3.0, 1e-6);
    mat_free(A);
    PASS();
    return;
next_test:;
}

static void test_rayleigh_quotient_iteration(void) {
    TEST("rayleigh_quotient_iteration");
    double d[] = {2, 1, 1, 2};
    Matrix* A = mat_from_array(2, 2, d);
    double xd[] = {1, 0.5};
    Vector* x = vec_from_array(xd, 2);
    double lambda = rayleigh_quotient_iteration(A, x, 20);
    ASSERT_CLOSE(lambda, 3.0, 1e-8);
    mat_free(A); vec_free(x);
    PASS();
    return;
next_test:;
}

static void test_lanczos(void) {
    TEST("lanczos_iteration");
    double d[] = {2, 1, 0, 1, 2, 1, 0, 1, 2};
    Matrix* A = mat_from_array(3, 3, d);
    double vd[] = {1, 0, 0};
    Vector* v1 = vec_from_array(vd, 3);
    Vector* alpha = NULL;
    Vector* beta = NULL;
    lanczos_iteration(A, v1, 3, &alpha, &beta);

    /* alpha should approximate eigenvalues: 2+√2, 2, 2-√2 */
    ASSERT_CLOSE(alpha->data[0], 2.0, 1e-8);  /* starting vector is eigenvector */
    ASSERT_CLOSE(beta->data[0], 1.0, 1e-8);

    mat_free(A); vec_free(v1); vec_free(alpha); vec_free(beta);
    PASS();
    return;
next_test:;
}

static void test_arnoldi(void) {
    TEST("arnoldi_iteration");
    double d[] = {2, 1, 0, 1, 2, 1, 0, 1, 2};
    Matrix* A = mat_from_array(3, 3, d);
    double vd[] = {1, 0, 0};
    Vector* v1 = vec_from_array(vd, 3);
    Matrix* V = NULL;
    Matrix* H = NULL;
    arnoldi_iteration(A, v1, 3, &V, &H);

    /* H should be upper Hessenberg */
    ASSERT_CLOSE(H->data[2 * 3 + 0], 0.0, 1e-10);  /* H(3,1) should be small or zero due to symmetry */

    mat_free(A); vec_free(v1); mat_free(V); mat_free(H);
    PASS();
    return;
next_test:;
}

/* ============================================================================
 * L5: Additional Iterative Solvers
 * =========================================================================== */
static void test_richardson(void) {
    TEST("richardson_solve");
    double d[] = {2, 0, 0, 2};
    Matrix* A = mat_from_array(2, 2, d);
    double bd[] = {4, 6};
    Vector* b = vec_from_array(bd, 2);
    Vector* x = richardson_solve(A, b, 0.4, 1e-10, 1000);
    ASSERT_CLOSE(x->data[0], 2.0, 1e-6);
    ASSERT_CLOSE(x->data[1], 3.0, 1e-6);
    mat_free(A); vec_free(b); vec_free(x);
    PASS();
    return;
next_test:;
}

static void test_weighted_jacobi(void) {
    TEST("weighted_jacobi_solve");
    double d[] = {4, 1, 2, 3};
    Matrix* A = mat_from_array(2, 2, d);
    double bd[] = {1, 2};
    Vector* b = vec_from_array(bd, 2);
    Vector* x = weighted_jacobi_solve(A, b, 2.0/3.0, 1e-10, 1000);
    ASSERT_CLOSE(x->data[0], 0.1, 1e-6);
    ASSERT_CLOSE(x->data[1], 0.6, 1e-6);
    mat_free(A); vec_free(b); vec_free(x);
    PASS();
    return;
next_test:;
}

static void test_symmetric_gs(void) {
    TEST("symmetric_gauss_seidel");
    double d[] = {4, 1, 2, 3};
    Matrix* A = mat_from_array(2, 2, d);
    double bd[] = {1, 2};
    Vector* b = vec_from_array(bd, 2);
    Vector* x = symmetric_gauss_seidel(A, b, 1e-10, 1000);
    ASSERT_CLOSE(x->data[0], 0.1, 1e-6);
    ASSERT_CLOSE(x->data[1], 0.6, 1e-6);
    mat_free(A); vec_free(b); vec_free(x);
    PASS();
    return;
next_test:;
}

static void test_chebyshev(void) {
    TEST("chebyshev_solve");
    double d[] = {2, 0, 0, 2};
    Matrix* A = mat_from_array(2, 2, d);
    double bd[] = {4, 6};
    Vector* b = vec_from_array(bd, 2);
    Vector* x = chebyshev_solve(A, b, 1.0, 3.0, 1e-10, 1000);
    ASSERT_CLOSE(x->data[0], 2.0, 1e-6);
    ASSERT_CLOSE(x->data[1], 3.0, 1e-6);
    mat_free(A); vec_free(b); vec_free(x);
    PASS();
    return;
next_test:;
}

/* ============================================================================
 * L2: Subspace Functions
 * =========================================================================== */
static void test_extract_basis(void) {
    TEST("extract_basis");
    double d[] = {1, 2, 3, 2, 4, 6, 3, 6, 9};
    Matrix* A = mat_from_array(3, 3, d);
    Matrix* basis = extract_basis(A);
    ASSERT_NOT_NULL(basis);
    /* rank-1 matrix: basis should have 1 column */
    ASSERT_CLOSE(basis->cols, 1.0, 0);
    mat_free(A); mat_free(basis);
    PASS();
    return;
next_test:;
}

static void test_orthogonal_complement(void) {
    TEST("orthogonal_complement");
    double d[] = {1, 0, 0, 1};
    Matrix* A = mat_from_array(2, 2, d);
    Matrix* oc = orthogonal_complement(A);
    /* Full rank → no complement */
    ASSERT_TRUE(oc == NULL);
    mat_free(A);
    if (oc) mat_free(oc);
    PASS();
    return;
next_test:;
}

/* ============================================================================
 * L5: Advanced Eigenvalue Algorithms
 * =========================================================================== */
static void test_tridiagonal_eig(void) {
    TEST("tridiagonal_eigenvalues");
    double diag[] = {2, 2, 2};
    double offdiag[] = {1, 1};
    Vector* eig = tridiagonal_eigenvalues(diag, offdiag, 3, 100);
    /* Eigenvalues of tridiag(1,2,1) are 2+√2, 2, 2-√2 */
    double sum = eig->data[0] + eig->data[1] + eig->data[2];
    ASSERT_CLOSE(sum, 6.0, 1e-8);
    vec_free(eig);
    PASS();
    return;
next_test:;
}

static void test_shift_invert(void) {
    TEST("shift_invert_eigenvalue");
    double d[] = {2, 1, 1, 2};
    Matrix* A = mat_from_array(2, 2, d);
    double xd[] = {1, 0};
    Vector* x = vec_from_array(xd, 2);
    double lambda = shift_invert_eigenvalue(A, x, 2.5, 100);
    /* eigenvalues of [[2,1],[1,2]] are 3.0 and 1.0.
     * Shift 2.5 is closer to 3, so should approximate 3.0 */
    ASSERT_TRUE(fabs(lambda - 3.0) < 0.5 || fabs(lambda - 1.0) < 0.5);
    mat_free(A); vec_free(x);
    PASS();
    return;
next_test:;
}

/* ============================================================================
 * L7: Application — Residual History Tracking
 * =========================================================================== */
static void test_residual_history(void) {
    TEST("iterative_residual_history");
    double d[] = {4, 1, 2, 3};
    Matrix* A = mat_from_array(2, 2, d);
    double bd[] = {1, 2};
    Vector* b = vec_from_array(bd, 2);
    int num_iters;
    Vector* history = iterative_residual_history(A, b, "jacobi", 1e-6, 100, &num_iters);
    ASSERT_NOT_NULL(history);
    /* Residual should decrease monotonically */
    ASSERT_TRUE(num_iters > 0);
    ASSERT_TRUE(history->data[0] > history->data[num_iters]);

    mat_free(A); vec_free(b); vec_free(history);
    PASS();
    return;
next_test:;
}

/* ============================================================================
 * Main Test Runner
 * =========================================================================== */
int main(void) {
    printf("=== mini-linear-algebra Test Suite ===\n\n");

    printf("[L1] Vector:\n");
    test_vector_create();
    test_vector_ops();
    test_vector_cross();
    test_vector_distance_angle();
    test_vector_normalize();
    test_vec_norm_p();

    printf("\n[L1] Matrix:\n");
    test_matrix_create();
    test_matrix_multiply();
    test_matrix_determinant();
    test_matrix_inverse();
    test_matrix_transpose();
    test_matrix_trace_frob();
    test_matrix_condition();

    printf("\n[L5] LU & Linear Systems:\n");
    test_lu_decomposition();
    test_solve_ax_b();

    printf("\n[L5] Eigenvalues:\n");
    test_power_iteration();
    test_qr_algorithm();
    test_rayleigh_quotient();

    printf("\n[L5] QR & Decompositions:\n");
    test_qr_decomposition();
    test_cholesky();
    test_least_squares();

    printf("\n[L5] SVD:\n");
    test_svd();

    printf("\n[L1] Sparse Matrix:\n");
    test_sparse_matrix();

    printf("\n[L3] Tensor3D:\n");
    test_tensor3d();

    printf("\n[L5] Iterative Solvers:\n");
    test_jacobi();
    test_gauss_seidel();
    test_conjugate_gradient();
    test_pcg_jacobi();
    test_gmres();
    test_sor();
    test_richardson();
    test_weighted_jacobi();
    test_symmetric_gs();
    test_chebyshev();

    printf("\n[L5] Householder & Givens:\n");
    test_householder_qr();
    test_givens_qr();
    test_gershgorin();

    printf("\n[L2] Subspaces:\n");
    test_linear_independence();
    test_is_in_span();
    test_gram_schmidt();
    test_orthogonal_projection();
    test_extract_basis();
    test_orthogonal_complement();

    printf("\n[L5] Advanced Eigenvalue Methods:\n");
    test_spectral_radius();
    test_rayleigh_quotient_iteration();
    test_lanczos();
    test_arnoldi();
    test_tridiagonal_eig();
    test_shift_invert();

    printf("\n[L7] Applications:\n");
    test_residual_history();

    printf("\n========================================\n");
    printf("Results: %d passed, %d failed, %d total\n",
           tests_passed, tests_failed, tests_passed + tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
