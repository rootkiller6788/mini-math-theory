#ifndef PROBABILITY_H
#define PROBABILITY_H

/* L1: Probability axioms */
double prob_union(double a, double b, double intersection);
double prob_intersection_independent(double a, double b);
double prob_conditional(double a_given_b_num, double b);
double prob_bayes(double prior, double likelihood, double evidence);
double prob_complement(double p);

/* L1: Combinatorics */
int    factorial(int n);
long long nCr(int n, int r);
long long nPr(int n, int r);
double factorial_approx(int n);
long long catalan(int n);
long long bell_number(int n);
long long stirling_s2(int n, int k);
long long derangement(int n);

/* L4: Law of Large Numbers verification */
double* llN_coin_flips(int n);
double* llN_uniform_mean(int n);

/* L4: Central Limit Theorem verification */
void clt_histogram(int n_per_sample, int n_samples, double* bins, int n_bins,
                   int* counts);

/* L5: Law of Total Probability */
double law_of_total_probability(double* p_a_given_b, double* p_b, int n_parts);
double extended_bayes(double* p_a_given_b, double* p_b, int n_parts, int k);

/* L5: Poisson Process */
int poisson_process_simulate(double lambda, double T, double* arrival_times,
                              int max_arrivals);
double poisson_process_prob_k(double lambda, double T, int k);

/* L8: Concentration inequalities */
double markov_bound(double expectation, double a);
double chebyshev_bound(double variance, double k_sigma);
double chernoff_bound_bernoulli(int n, double p, double delta);
double hoeffding_bound(int n, double t, double a, double b);

/* L5: Random permutations */
void fisher_yates_shuffle(int* arr, int n);
int* random_permutation(int n);

#endif
