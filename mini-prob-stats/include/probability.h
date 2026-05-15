#ifndef PROBABILITY_H
#define PROBABILITY_H

double prob_union(double a, double b, double intersection);
double prob_intersection_independent(double a, double b);
double prob_conditional(double a_given_b_num, double b);
double prob_bayes(double prior, double likelihood, double evidence);
double prob_complement(double p);
int    factorial(int n);
long long nCr(int n, int r);
long long nPr(int n, int r);

#endif
