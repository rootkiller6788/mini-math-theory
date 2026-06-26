#ifndef MONTE_CARLO_H
#define MONTE_CARLO_H

/* L5: Monte Carlo integration */
double mc_integrate(double (*f)(double), double a, double b, int n_samples);
double mc_pi(int n_samples);
double mc_importance_sampling(double (*f)(double), double (*g)(double),
                              double (*g_sample)(void), int n_samples);

/* L5: MCMC */
void mcmc_metropolis(double (*target)(double), double start, int n_iter,
                     double* samples, double proposal_sd);
void mcmc_gibbs_bivariate_normal(int n_iter, double rho, double* samples_x,
                                  double* samples_y);

/* L8: Variance reduction techniques */
double mc_antithetic_integrate(double (*f)(double), double a, double b,
                                int n_samples);
double mc_control_variate_integrate(double (*f)(double), double a, double b,
                                     int n_samples, double true_mean_h);
double mc_stratified_integrate(double (*f)(double), double a, double b,
                                int n_samples, int n_strata);

/* L7: Applications */
double mc_portfolio_var(double* weights, double* returns, double* volatilities,
                         double* correlation, int n_assets, int n_sim,
                         double alpha);
double mc_power_t_test(int n_sim, int n, double mu0, double mu1, double sigma,
                        double alpha);

#endif
