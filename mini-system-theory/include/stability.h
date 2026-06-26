#ifndef STABILITY_H
#define STABILITY_H

#include <stdbool.h>

bool is_stable_poles(double* eigenvalues, int n);
bool is_bibo_stable(double* impulse_response, int n, double tol);

double* lyapunov_solve(double** A, int n, double** Q);

void bode_margins(double* num, int n_num, double* den, int n_den,
                  double* gain_margin, double* phase_margin);

double* root_locus_eig(double** A, double** B, double** C, int n, double K);

/* Additional stability analysis APIs */
int stability_nyquist_check(double* num, int num_len, double* den, int den_len);
int stability_routh_hurwitz(double* coeffs, int degree);
int stability_kharitonov_test(double* coeffs_min, double* coeffs_max, int degree);
void stability_margins(double* num, int num_len, double* den, int den_len,
                       double* gm_db, double* pm_deg);
void stability_lyapunov_solve_direct(double** A, double** Q, int n, double** X);

#endif
