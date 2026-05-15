#include "mutual_information.h"
#include "entropy.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(void) {
    printf("====== Mutual Information Demo ======\n\n");

    printf("--- 1. Perfect Dependence: I(X;X) = H(X) ---\n");
    double** pxx = malloc(3 * sizeof(double*));
    for (int i = 0; i < 3; i++) pxx[i] = calloc(3, sizeof(double));
    pxx[0][0] = 0.3; pxx[1][1] = 0.5; pxx[2][2] = 0.2;
    double px[3] = {0.3, 0.5, 0.2};
    double py[3] = {0.3, 0.5, 0.2};
    double mi_self = mutual_information((const double**)pxx, px, py, 3, 3);
    double hx = entropy_shannon(px, 3);
    printf("I(X;X) = %.4f, H(X) = %.4f → %s\n\n",
           mi_self, hx, fabs(mi_self - hx) < 1e-10 ? "EQUAL ✓" : "DIFF?");

    printf("--- 2. Independence: I(X;Y) = 0 ---\n");
    double** p_ind = malloc(2 * sizeof(double*));
    for (int i = 0; i < 2; i++) p_ind[i] = calloc(2, sizeof(double));
    p_ind[0][0] = 0.25; p_ind[0][1] = 0.25;
    p_ind[1][0] = 0.25; p_ind[1][1] = 0.25;
    double pxi[2] = {0.5, 0.5}, pyi[2] = {0.5, 0.5};
    double mi_ind = mutual_information((const double**)p_ind, pxi, pyi, 2, 2);
    printf("Independent X,Y: I(X;Y) = %.6f ≈ 0\n\n", mi_ind);

    printf("--- 3. Partial Dependence ---\n");
    double** p_xy = malloc(2 * sizeof(double*));
    for (int i = 0; i < 2; i++) p_xy[i] = calloc(3, sizeof(double));
    p_xy[0][0] = 0.3; p_xy[0][1] = 0.1; p_xy[0][2] = 0.1; // p(X=0) = 0.5
    p_xy[1][0] = 0.1; p_xy[1][1] = 0.3; p_xy[1][2] = 0.1; // p(X=1) = 0.5
    double px2[2] = {0.5, 0.5}, py2[3] = {0.4, 0.4, 0.2};
    double mi_p = mutual_information((const double**)p_xy, px2, py2, 2, 3);
    printf("I(X;Y) = %.4f bits\n", mi_p);
    printf("H(X) = %.4f, H(Y) = %.4f\n",
           entropy_shannon(px2, 2), entropy_shannon(py2, 3));
    printf("H(X,Y) = %.4f\n", entropy_joint((const double**)p_xy, 2, 3));
    printf("H(Y|X) = %.4f\n", entropy_conditional((const double**)p_xy, 2, 3));
    printf("\n");

    printf("--- 4. Normalized MI ---\n");
    double nmi = normalized_mutual_information((const double**)p_xy, px2, py2, 2, 3);
    printf("NMI(X;Y) = %.4f (0=independent, 1=identical)\n", nmi);
    printf("→ partial information overlap detected\n");

    for (int i = 0; i < 3; i++) free(pxx[i]); free(pxx);
    for (int i = 0; i < 2; i++) { free(p_ind[i]); free(p_xy[i]); }
    free(p_ind); free(p_xy);
    return 0;
}
