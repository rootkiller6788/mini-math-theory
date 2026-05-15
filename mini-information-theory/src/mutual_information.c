#include "mutual_information.h"
#include "entropy.h"
#include <math.h>
#include <stdlib.h>

double mutual_information(const double** pxy, const double* px, const double* py,
                          int nx, int ny) {
    double mi = 0.0;
    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
            if (pxy[i][j] > 0.0)
                mi += pxy[i][j] * log2(pxy[i][j] / (px[i] * py[j]));
    return mi;
}

double normalized_mutual_information(const double** pxy, const double* px,
                                     const double* py, int nx, int ny) {
    double mi = mutual_information(pxy, px, py, nx, ny);
    double hx = 0.0, hy = 0.0;
    for (int i = 0; i < nx; i++) if (px[i] > 0) hx -= px[i] * log2(px[i]);
    for (int j = 0; j < ny; j++) if (py[j] > 0) hy -= py[j] * log2(py[j]);
    double denom = (hx + hy) / 2.0;
    return denom > 0 ? mi / denom : 0.0;
}

void mutual_info_from_counts(const int** counts, int nx, int ny, double* mi) {
    int total = 0;
    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
            total += counts[i][j];
    if (total == 0) { *mi = 0; return; }

    double** pxy = malloc(nx * sizeof(double*));
    double* px = calloc(nx, sizeof(double));
    double* py = calloc(ny, sizeof(double));

    for (int i = 0; i < nx; i++) {
        pxy[i] = malloc(ny * sizeof(double));
        for (int j = 0; j < ny; j++) {
            pxy[i][j] = (double)counts[i][j] / total;
            px[i] += pxy[i][j];
            py[j] += pxy[i][j];
        }
    }

    *mi = mutual_information((const double**)pxy, px, py, nx, ny);

    for (int i = 0; i < nx; i++) free(pxy[i]);
    free(pxy); free(px); free(py);
}
