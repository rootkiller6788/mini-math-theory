#include "stability.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static double** mat_create(int r, int c) {
    double** m = (double**)malloc(r * sizeof(double*));
    for (int i = 0; i < r; i++)
        m[i] = (double*)calloc(c, sizeof(double));
    return m;
}

static void mat_free(double** m, int r) {
    for (int i = 0; i < r; i++) free(m[i]);
    free(m);
}

int main(void) {
    printf("=== Stability Analysis Demo ===\n\n");

    double evals_stable[] = { -1.0, 0.0,  2.0, 0.0,  -3.0, 0.0,  -4.0, 0.0 };
    printf("Matrix [[-1,2],[-3,-4]] -> eigenvalues via char. poly:\n");
    double A1[2][2] = {{-1, 2}, {-3, -4}};
    double trace1 = A1[0][0] + A1[1][1];
    double det1 = A1[0][0] * A1[1][1] - A1[0][1] * A1[1][0];
    double disc1 = trace1 * trace1 - 4 * det1;
    double e1[4] = {0, 0, 0, 0};
    if (disc1 >= 0) {
        e1[0] = (trace1 + sqrt(disc1)) / 2.0;
        e1[2] = (trace1 - sqrt(disc1)) / 2.0;
    } else {
        e1[0] = trace1 / 2.0;
        e1[1] = sqrt(-disc1) / 2.0;
        e1[2] = trace1 / 2.0;
        e1[3] = -sqrt(-disc1) / 2.0;
    }
    for (int i = 0; i < 2; i++)
        printf("  %.6f %+.6fi\n", e1[2 * i], e1[2 * i + 1]);
    printf("  Stable: %s\n\n", is_stable_poles(e1, 2) ? "Yes" : "No");

    printf("Matrix [[1,0],[0,-1]] -> eigenvalues:\n");
    double A2[2][2] = {{1, 0}, {0, -1}};
    double e2[] = {1.0, 0.0, -1.0, 0.0};
    printf("  %.6f %+.6fi\n", e2[0], e2[1]);
    printf("  %.6f %+.6fi\n", e2[2], e2[3]);
    printf("  Stable: %s\n\n", is_stable_poles(e2, 2) ? "Yes" : "No");

    printf("Lyapunov Equation: A*X + X*A' = -Q\n");
    printf("  A = [[-2, 1], [0, -3]], Q = I\n");
    double** A = mat_create(2, 2);
    double** Q = mat_create(2, 2);
    A[0][0] = -2.0; A[0][1] = 1.0;
    A[1][0] = 0.0;  A[1][1] = -3.0;
    Q[0][0] = 1.0;  Q[0][1] = 0.0;
    Q[1][0] = 0.0;  Q[1][1] = 1.0;

    double* X_vec = lyapunov_solve(A, 2, Q);
    printf("  Solution X:\n");
    printf("    [%10.6f  %10.6f]\n", X_vec[0], X_vec[1]);
    printf("    [%10.6f  %10.6f]\n", X_vec[2], X_vec[3]);

    printf("  Verify: A*X + X*A' =\n");
    double AX_XAt[4];
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++) {
            double s = 0;
            for (int k = 0; k < 2; k++)
                s += A[i][k] * X_vec[k * 2 + j];
            s += X_vec[i * 2 + j] * A[j][j];
            for (int k = 0; k < 2; k++)
                if (k != j) s += X_vec[i * 2 + k] * A[j][k];
            AX_XAt[i * 2 + j] = s;
        }
    printf("    [%10.6f  %10.6f]\n", AX_XAt[0], AX_XAt[1]);
    printf("    [%10.6f  %10.6f]\n\n", AX_XAt[2], AX_XAt[3]);

    printf("BIBO Stability:\n");
    double imp_stable[] = {1.0, 0.5, 0.25, 0.125, 0.0625, 0.03125,
                           0.015625, 0.0078125, 0.00390625, 0.001953125};
    int n_imp = 10;
    double sum_abs = 0;
    for (int i = 0; i < n_imp; i++) sum_abs += fabs(imp_stable[i]);
    printf("  Decaying impulse (sum|h| = %.6f): BIBO %s\n",
           sum_abs, is_bibo_stable(imp_stable, n_imp, 0.01) ? "Stable" : "Unstable");

    double imp_unstable[] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    sum_abs = 0;
    for (int i = 0; i < 10; i++) sum_abs += fabs(imp_unstable[i]);
    printf("  Constant impulse (sum|h| = %.1f): BIBO %s\n",
           sum_abs, is_bibo_stable(imp_unstable, 10, 0.01) ? "Stable" : "Unstable");

    double imp_grow[] = {1.0, 2.0, 4.0, 8.0, 16.0};
    sum_abs = 0;
    for (int i = 0; i < 5; i++) sum_abs += fabs(imp_grow[i]);
    printf("  Growing impulse (sum|h| = %.0f): BIBO %s\n",
           sum_abs, is_bibo_stable(imp_grow, 5, 0.01) ? "Stable" : "Unstable");

    printf("\nBode Margins (1st-order: 1/(s+1)):\n");
    double num[] = {1.0};
    double den[] = {1.0, 1.0};
    double gm, pm;
    bode_margins(num, 1, den, 2, &gm, &pm);
    printf("  Gain margin: %.2f dB\n", gm);
    printf("  Phase margin: %.2f degrees\n", pm);

    printf("\nRoot Locus (K=2):\n");
    double** Ar = mat_create(2, 2);
    double** Br = mat_create(2, 1);
    double** Cr = mat_create(1, 2);
    Ar[0][0] = 0; Ar[0][1] = 1;
    Ar[1][0] = -2; Ar[1][1] = -3;
    Br[0][0] = 0; Br[1][0] = 1;
    Cr[0][0] = 1; Cr[0][1] = 0;
    double* rl_eig = root_locus_eig(Ar, Br, Cr, 2, 2.0);
    printf("  Closed-loop eigenvalues for K=2:\n");
    for (int i = 0; i < 2; i++)
        printf("    %.6f %+.6fi\n", rl_eig[2 * i], rl_eig[2 * i + 1]);

    free(rl_eig);
    free(X_vec);
    mat_free(A, 2);
    mat_free(Q, 2);
    mat_free(Ar, 2);
    mat_free(Br, 2);
    mat_free(Cr, 1);

    printf("\nDone.\n");
    return 0;
}
