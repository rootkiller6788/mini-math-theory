#include "dp.h"
#include <stdlib.h>
#include <string.h>

static int max(int a, int b) { return a > b ? a : b; }
static int min(int a, int b) { return a < b ? a : b; }

int knapsack_01(int* weights, int* values, int n, int capacity) {
    int* dp = (int*)calloc(capacity + 1, sizeof(int));
    for (int i = 0; i < n; i++) {
        for (int w = capacity; w >= weights[i]; w--) {
            if (dp[w - weights[i]] + values[i] > dp[w]) {
                dp[w] = dp[w - weights[i]] + values[i];
            }
        }
    }
    int result = dp[capacity];
    free(dp);
    return result;
}

int lcs_length(const char* a, const char* b) {
    int m = (int)strlen(a);
    int n = (int)strlen(b);
    int* prev = (int*)calloc(n + 1, sizeof(int));
    int* curr = (int*)calloc(n + 1, sizeof(int));
    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (a[i - 1] == b[j - 1]) {
                curr[j] = prev[j - 1] + 1;
            } else {
                curr[j] = max(prev[j], curr[j - 1]);
            }
        }
        int* temp = prev;
        prev = curr;
        curr = temp;
    }
    int result = prev[n];
    free(prev);
    free(curr);
    return result;
}

int edit_distance(const char* a, const char* b) {
    int m = (int)strlen(a);
    int n = (int)strlen(b);
    if (m < n) {
        const char* temp = a; a = b; b = temp;
        int t = m; m = n; n = t;
    }
    int* prev = (int*)malloc(sizeof(int) * (n + 1));
    int* curr = (int*)malloc(sizeof(int) * (n + 1));
    for (int j = 0; j <= n; j++) prev[j] = j;
    for (int i = 1; i <= m; i++) {
        curr[0] = i;
        for (int j = 1; j <= n; j++) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            curr[j] = min(min(prev[j] + 1, curr[j - 1] + 1), prev[j - 1] + cost);
        }
        int* temp = prev;
        prev = curr;
        curr = temp;
    }
    int result = prev[n];
    free(prev);
    free(curr);
    return result;
}

int rod_cutting(int* prices, int n) {
    int* dp = (int*)calloc(n + 1, sizeof(int));
    for (int i = 1; i <= n; i++) {
        int best = 0;
        for (int j = 0; j < i; j++) {
            best = max(best, prices[j] + dp[i - 1 - j]);
        }
        dp[i] = best;
    }
    int result = dp[n];
    free(dp);
    return result;
}

int coin_change(int* coins, int ncoins, int amount) {
    int* dp = (int*)calloc(amount + 1, sizeof(int));
    dp[0] = 1;
    for (int i = 0; i < ncoins; i++) {
        for (int a = coins[i]; a <= amount; a++) {
            dp[a] += dp[a - coins[i]];
        }
    }
    int result = dp[amount];
    free(dp);
    return result;
}

int coin_change_min_coins(int* coins, int ncoins, int amount) {
    int* dp = (int*)malloc(sizeof(int) * (amount + 1));
    for (int i = 0; i <= amount; i++) dp[i] = amount + 1;
    dp[0] = 0;
    for (int a = 1; a <= amount; a++) {
        for (int i = 0; i < ncoins; i++) {
            if (coins[i] <= a && dp[a - coins[i]] + 1 < dp[a]) {
                dp[a] = dp[a - coins[i]] + 1;
            }
        }
    }
    int result = (dp[amount] > amount) ? -1 : dp[amount];
    free(dp);
    return result;
}

long long fibonacci_dp(int n) {
    if (n <= 1) return n;
    long long prev2 = 0, prev1 = 1;
    for (int i = 2; i <= n; i++) {
        long long curr = prev1 + prev2;
        prev2 = prev1;
        prev1 = curr;
    }
    return prev1;
}

int max_subarray(int* arr, int n) {
    int best = arr[0];
    int current = arr[0];
    for (int i = 1; i < n; i++) {
        current = max(arr[i], current + arr[i]);
        best = max(best, current);
    }
    return best;
}
