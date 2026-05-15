#ifndef STABILITY_H
#define STABILITY_H

#include <stdbool.h>

bool is_stable_poles(double* eigenvalues, int n);
bool is_bibo_stable(double* impulse_response, int n, double tol);

double* lyapunov_solve(double** A, int n, double** Q);

void bode_margins(double* num, int n_num, double* den, int n_den,
                  double* gain_margin, double* phase_margin);

double* root_locus_eig(double** A, double** B, double** C, int n, double K);

#endif
