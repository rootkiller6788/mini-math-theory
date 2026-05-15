#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "dp.h"

static int fib_naive(int n) {
    if (n <= 1) return n;
    return fib_naive(n - 1) + fib_naive(n - 2);
}

int main(void)
{
    clock_t start, end;
    double elapsed;

    printf("=== Dynamic Programming Demo ===\n\n");

    printf("--- Fibonacci ---\n");
    start = clock();
    int fib_naive_10 = fib_naive(10);
    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    printf("fib_naive(10)  = %d (time: %.3f ms)\n", fib_naive_10, elapsed);
    printf("Expected: 55\n");

    start = clock();
    long long fib_dp_10 = fibonacci_dp(10);
    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    printf("fib_dp(10)     = %lld (time: %.3f ms)\n", fib_dp_10, elapsed);
    printf("Expected: 55\n\n");

    start = clock();
    int fib_naive_30 = fib_naive(30);
    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    printf("fib_naive(30)  = %d (time: %.3f ms)\n", fib_naive_30, elapsed);
    printf("Expected: 832040\n");

    start = clock();
    long long fib_dp_30 = fibonacci_dp(30);
    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    printf("fib_dp(30)     = %lld (time: %.3f ms)\n", fib_dp_30, elapsed);
    printf("Expected: 832040\n\n");

    printf("--- 0/1 Knapsack ---\n");
    int wt[]  = {2, 3, 4, 5};
    int val[] = {3, 4, 5, 6};
    int cap = 8;
    int n_knap = 4;
    int kp_result = knapsack_01(wt, val, n_knap, cap);
    printf("weights = [2,3,4,5], values = [3,4,5,6], capacity = 8\n");
    printf("Max value: %d\n", kp_result);
    printf("Expected: 10 (items with weights 3+5 and values 4+6)\n\n");

    printf("--- Longest Common Subsequence ---\n");
    const char *X = "AGGTAB";
    const char *Y = "GXTXAYB";
    int lcs_len = lcs_length(X, Y);
    printf("LCS(\"%s\", \"%s\") = %d\n", X, Y, lcs_len);
    printf("Expected: 4 (\"GTAB\")\n\n");

    printf("--- Edit Distance (Levenshtein) ---\n");
    const char *s1 = "kitten";
    const char *s2 = "sitting";
    int ed = edit_distance(s1, s2);
    printf("edit_distance(\"%s\", \"%s\") = %d\n", s1, s2, ed);
    printf("Expected: 3 (k->s, e->i, +g)\n\n");

    printf("--- Rod Cutting ---\n");
    int prices[] = {1, 5, 8, 9, 10, 17, 17, 20};
    int rod_len = 8;
    int rod_val = rod_cutting(prices, rod_len);
    printf("prices = [1,5,8,9,10,17,17,20], length = %d\n", rod_len);
    printf("Max revenue: %d\n", rod_val);
    printf("Expected: 22 (two pieces of length 6+2, i.e. 17+5)\n\n");

    printf("--- Coin Change (number of ways) ---\n");
    int coins[] = {1, 2, 5};
    int amount1 = 11;
    int ways = coin_change(coins, 3, amount1);
    printf("coins = [1,2,5], amount = %d\n", amount1);
    printf("Number of ways: %d\n", ways);
    printf("Expected: 11\n\n");

    printf("--- Coin Change (minimum coins) ---\n");
    int min_coins = coin_change_min_coins(coins, 3, amount1);
    printf("coins = [1,2,5], amount = %d\n", amount1);
    printf("Minimum coins: %d\n", min_coins);
    printf("Expected: 3 (5+5+1)\n\n");

    printf("--- Maximum Subarray (Kadane's Algorithm) ---\n");
    int subarr[] = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    int subarr_len = 9;
    int max_sub = max_subarray(subarr, subarr_len);
    printf("arr = [-2,1,-3,4,-1,2,1,-5,4]\n");
    printf("Max subarray sum: %d\n", max_sub);
    printf("Expected: 6 (subarray [4,-1,2,1])\n");

    return 0;
}
