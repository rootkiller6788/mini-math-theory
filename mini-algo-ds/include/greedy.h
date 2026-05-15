#ifndef GREEDY_H
#define GREEDY_H

int activity_selection(int* start, int* finish, int n, int* selected);
double fractional_knapsack(int* weights, int* values, int n, int capacity, double* fractions);
int min_platforms(int* arrival, int* departure, int n);
int huffman_coding_cost(int* freqs, int n);

#endif
