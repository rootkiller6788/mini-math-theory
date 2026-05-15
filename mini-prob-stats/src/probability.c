#include "probability.h"

double prob_union(double a, double b, double intersection) {
    return a + b - intersection;
}

double prob_intersection_independent(double a, double b) {
    return a * b;
}

double prob_conditional(double a_given_b_num, double b) {
    if (b == 0.0) return 0.0;
    return a_given_b_num / b;
}

double prob_bayes(double prior, double likelihood, double evidence) {
    if (evidence == 0.0) return 0.0;
    return (likelihood * prior) / evidence;
}

double prob_complement(double p) {
    return 1.0 - p;
}

int factorial(int n) {
    if (n < 0) return 0;
    if (n > 12) return 0;
    int result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

long long nCr(int n, int r) {
    if (r < 0 || n < 0 || r > n) return 0;
    if (r > n - r) r = n - r;
    long long result = 1;
    for (int i = 1; i <= r; i++) {
        result = result * (n - r + i) / i;
    }
    return result;
}

long long nPr(int n, int r) {
    if (r < 0 || n < 0 || r > n) return 0;
    long long result = 1;
    for (int i = 0; i < r; i++) {
        result *= (n - i);
    }
    return result;
}
