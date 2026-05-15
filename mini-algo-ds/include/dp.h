#ifndef DP_H
#define DP_H

int knapsack_01(int* weights, int* values, int n, int capacity);
int lcs_length(const char* a, const char* b);
int edit_distance(const char* a, const char* b);
int rod_cutting(int* prices, int n);
int coin_change(int* coins, int ncoins, int amount);
int coin_change_min_coins(int* coins, int ncoins, int amount);
long long fibonacci_dp(int n);
int max_subarray(int* arr, int n);

#endif
