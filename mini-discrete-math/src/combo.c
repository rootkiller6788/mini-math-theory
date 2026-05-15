#include "combo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long long factorial(int n) {
    long long result = 1;
    for (int i = 2; i <= n; i++) result *= i;
    return result;
}

long long permutation(int n, int r) {
    if (r > n) return 0;
    long long result = 1;
    for (int i = n - r + 1; i <= n; i++) result *= i;
    return result;
}

long long combination(int n, int r) {
    if (r > n || r < 0) return 0;
    if (r > n - r) r = n - r;
    long long result = 1;
    for (int i = 0; i < r; i++) {
        result = result * (n - i) / (i + 1);
    }
    return result;
}

long long binomial_coefficient(int n, int k) {
    return combination(n, k);
}

void pascal_triangle(int n, long long** triangle) {
    for (int i = 0; i < n; i++) {
        triangle[i][0] = 1;
        triangle[i][i] = 1;
        for (int j = 1; j < i; j++)
            triangle[i][j] = triangle[i - 1][j - 1] + triangle[i - 1][j];
    }
}

static void swap(int* a, int* b) {
    int t = *a; *a = *b; *b = t;
}

// Heap's algorithm
static void perm_heaps(int* arr, int n, int k, void (*callback)(int*, int)) {
    if (k == 1) {
        callback(arr, n);
        return;
    }
    for (int i = 0; i < k; i++) {
        perm_heaps(arr, n, k - 1, callback);
        if (k % 2 == 0)
            swap(&arr[i], &arr[k - 1]);
        else
            swap(&arr[0], &arr[k - 1]);
    }
}

void permutations_generate(int* arr, int n, void (*callback)(int*, int)) {
    perm_heaps(arr, n, n, callback);
}

static void comb_rec(int* arr, int n, int r, int start, int* buf, int buf_idx,
                     void (*callback)(int*, int)) {
    if (buf_idx == r) {
        callback(buf, r);
        return;
    }
    for (int i = start; i < n; i++) {
        buf[buf_idx] = arr[i];
        comb_rec(arr, n, r, i + 1, buf, buf_idx + 1, callback);
    }
}

void combinations_generate(int* arr, int n, int r, void (*callback)(int*, int)) {
    int* buf = malloc(r * sizeof(int));
    comb_rec(arr, n, r, 0, buf, 0, callback);
    free(buf);
}

void stirling_numbers_second(int n, int k, long long* result) {
    long long dp[n + 1][k + 1];
    memset(dp, 0, sizeof(dp));
    dp[0][0] = 1;
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= k; j++)
            dp[i][j] = dp[i - 1][j - 1] + j * dp[i - 1][j];
    *result = dp[n][k];
}

long long catalan_number(int n) {
    return combination(2 * n, n) / (n + 1);
}

long long partitions_of_integer(int n) {
    long long dp[n + 1];
    memset(dp, 0, sizeof(dp));
    dp[0] = 1;
    for (int i = 1; i <= n; i++)
        for (int j = i; j <= n; j++)
            dp[j] += dp[j - i];
    return dp[n];
}

void partitions_enumerate(int n, int max_part, int* buf, int buf_idx,
                          void (*callback)(int*, int)) {
    if (n == 0) {
        callback(buf, buf_idx);
        return;
    }
    for (int p = (max_part < n ? max_part : n); p >= 1; p--) {
        buf[buf_idx] = p;
        partitions_enumerate(n - p, p, buf, buf_idx + 1, callback);
    }
}

long long derangements(int n) {
    if (n == 0) return 1;
    if (n == 1) return 0;
    long long d0 = 1, d1 = 0, d2;
    for (int i = 2; i <= n; i++) {
        d2 = (i - 1) * (d1 + d0);
        d0 = d1;
        d1 = d2;
    }
    return d1;
}
