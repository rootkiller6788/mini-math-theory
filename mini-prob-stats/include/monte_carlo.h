#ifndef MONTE_CARLO_H
#define MONTE_CARLO_H

double mc_integrate(double (*f)(double), double a, double b, int n_samples);
double mc_pi(int n_samples);
double mc_importance_sampling(double (*f)(double), double (*g)(double),
                              double (*g_sample)(void), int n_samples);
void mcmc_metropolis(double (*target)(double), double start, int n_iter,
                     double* samples, double proposal_sd);

#endif
