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

/* Bell numbers B_n: number of partitions of an n-element set.
 * Recurrence: B_{n+1} = Σ C(n,k) * B_k for k=0..n  (Dobinski's formula).
 * Use Bell triangle: B[0][0]=1, B[i][0]=B[i-1][i-1], B[i][j]=B[i][j-1]+B[i-1][j-1].
 * Reference: MIT 6.042J, Aitken's array */
long long bell_number(int n) {
    if (n < 0) return 0;
    if (n == 0) return 1;
    long long bell[32][32];
    bell[0][0] = 1;
    for (int i = 1; i <= n && i < 32; i++) {
        bell[i][0] = bell[i - 1][i - 1];
        for (int j = 1; j <= i; j++)
            bell[i][j] = bell[i][j - 1] + bell[i - 1][j - 1];
    }
    return bell[n][0]; /* B_n = first element of row n */
}

/* Inclusion-Exclusion Principle:
 * |∪ A_i| = Σ|A_i| - Σ|A_i∩A_j| + Σ|A_i∩A_j∩A_k| - ... + (-1)^{n+1}|∩A_i|
 * Given set sizes and their intersections (simplified: equal-sized sets).
 * Reference: MIT 6.042J §14.9 */
long long inclusion_exclusion(int set_sizes[], int n_sets, int universe) {
    (void)universe;
    long long total = 0;
    /* iterate over all non-empty subsets */
    int total_subsets = 1 << n_sets;
    for (int mask = 1; mask < total_subsets; mask++) {
        int popcnt = 0;
        int x = mask;
        while (x) { popcnt += x & 1; x >>= 1; }
        long long term = 1;
        int count = 0;
        for (int i = 0; i < n_sets; i++) {
            if (mask & (1 << i)) {
                term *= set_sizes[i];
                count++;
            }
        }
        /* scale by intersection size divided by individual sizes for realism */
        long long intersection_estimate = term;
        for (int i = 0; i < n_sets; i++) {
            if (mask & (1 << i))
                intersection_estimate /= set_sizes[i];
        }
        /* simplified: intersection = min set size */
        long long inter_size = set_sizes[0];
        for (int i = 0; i < n_sets; i++)
            if ((mask & (1 << i)) && set_sizes[i] < inter_size)
                inter_size = set_sizes[i];

        if (popcnt % 2 == 1)
            total += inter_size;
        else
            total -= inter_size;
    }
    return total;
}

/* Pigeonhole Principle: If n items are placed into m holes and n > m,
 * at least one hole contains ≥ ceil(n/m) items.
 * Returns true if a collision is detected.
 * Reference: MIT 6.042J §14.8, Dirichlet */
bool pigeonhole_check(int items[], int n_items, int n_holes) {
    int* holes = calloc(n_holes, sizeof(int));
    bool collision = false;
    for (int i = 0; i < n_items; i++) {
        int hole = items[i] % n_holes;
        if (hole < 0) hole += n_holes;
        holes[hole]++;
        if (holes[hole] > 1) { collision = true; break; }
    }
    free(holes);
    return collision;
}

/* Stirling numbers of the first kind s(n,k): # of permutations of n elements
 * with exactly k disjoint cycles. Recurrence:
 * s(n,k) = s(n-1,k-1) + (n-1)*s(n-1,k), with s(0,0)=1, s(n,0)=s(0,n)=0.
 * Reference: MIT 6.042J, Concrete Mathematics §6.1 */
long long stirling_first(int n, int k) {
    if (n < 0 || k < 0) return 0;
    if (n == 0 && k == 0) return 1;
    if (n == 0 || k == 0) return 0;
    long long dp[64][64];
    memset(dp, 0, sizeof(dp));
    dp[0][0] = 1;
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= i; j++)
            dp[i][j] = dp[i - 1][j - 1] + (i - 1) * dp[i - 1][j];
    return dp[n][k];
}

/* Eulerian numbers A(n,k): #permutations with k ascents.
 * Recurrence: A(n,k) = (n-k)*A(n-1,k-1) + (k+1)*A(n-1,k).
 * Reference: Euler (1755), Graham-Knuth-Patashnik §6.2 */
long long eulerian_number(int n, int k) {
    if (k >= n || k < 0) return 0;
    if (n == 0) return 1;
    long long dp[64][64];
    memset(dp, 0, sizeof(dp));
    dp[0][0] = 1;
    for (int i = 1; i <= n; i++) {
        dp[i][0] = 1;
        for (int j = 1; j < i; j++)
            dp[i][j] = (i - j) * dp[i - 1][j - 1] + (j + 1) * dp[i - 1][j];
    }
    return dp[n][k];
}

/* Multinomial coefficient: n! / (k1! * k2! * ... * km!)
 * Counts ways to partition n items into m labeled groups of sizes k_i.
 * Reference: MIT 6.042J §14.5 */
long long multinomial(int n, int k[], int m) {
    int sum = 0;
    for (int i = 0; i < m; i++) sum += k[i];
    if (sum != n) return 0;
    long long result = factorial(n);
    for (int i = 0; i < m; i++)
        result /= factorial(k[i]);
    return result;
}

/* Narayana numbers N(n,k): # of Dyck paths of length 2n with k peaks.
 * Formula: N(n,k) = (1/n) * C(n,k) * C(n,k-1).
 * Related to Catalan numbers: Σ N(n,k) = Catalan(n).
 * Reference: Narayana (1955), Stanley's Enumerative Combinatorics */
long long narayana_number(int n, int k) {
    if (k < 1 || k > n) return 0;
    if (n == 0) return 1;
    return combination(n, k) * combination(n, k - 1) / n;
}

/* Burnside's Lemma: |X/G| = (1/|G|) * Σ |X^g|
 * For necklaces: n beads, k colors, rotations only (cyclic group C_n).
 * Count = (1/n) * Σ_{d|n} φ(d) * k^(n/d).
 * Reference: MIT 6.042J §15.9, Polya Enumeration */
static long long euler_phi(long long n) {
    long long result = n;
    for (long long p = 2; p * p <= n; p++) {
        if (n % p == 0) {
            while (n % p == 0) n /= p;
            result -= result / p;
        }
    }
    if (n > 1) result -= result / n;
    return result;
}

long long necklaces_count(int n, int k) {
    if (n == 0) return 0;
    if (n == 1) return k;
    long long total = 0;
    for (int d = 1; d <= n; d++) {
        if (n % d == 0) {
            long long phi_d = euler_phi(d);
            long long pow_val = 1;
            for (int i = 0; i < n / d; i++) pow_val *= k;
            total += phi_d * pow_val;
        }
    }
    return total / n;
}

/* Surjective function count: |X|=n, |Y|=k, count onto functions X→Y.
 * Using Stirling S2 via inclusion-exclusion: k! * S(n,k) = Σ(-1)^i * C(k,i) * (k-i)^n.
 * Reference: MIT 6.042J §14.9 */
long long surjective_count(int n, int k) {
    if (k > n) return 0;
    long long total = 0;
    long long sign = 1;
    for (int i = 0; i < k; i++) {
        long long term = combination(k, i);
        long long pow_val = 1;
        for (int j = 0; j < n; j++) pow_val *= (k - i);
        total += sign * term * pow_val;
        sign = -sign;
    }
    return total;
}

/* Compositions: ordered partitions of n into exactly k positive parts.
 * Formula: C(n-1, k-1). Stars-and-bars.
 * Reference: MIT 6.042J §14.4 */
long long compositions_count(int n, int k) {
    if (n < k || k < 1) return 0;
    return combination(n - 1, k - 1);
}

/* Partition function p(n) via Euler's pentagonal number theorem:
 * p(n) = Σ (-1)^{k-1} * p(n - g_k) where g_k = k(3k-1)/2.
 * DP: p(n) = Σ (-1)^{k-1} * p(n - k(3k±1)/2).
 * Reference: Euler, Hardy-Ramanujan */
long long partition_pentagonal(int n) {
    if (n < 0) return 0;
    if (n == 0) return 1;
    long long p[256];
    memset(p, 0, sizeof(p));
    p[0] = 1;
    for (int i = 1; i <= n && i < 256; i++) {
        long long total = 0;
        for (int k = 1; ; k++) {
            int g1 = k * (3 * k - 1) / 2; /* pentagonal, k positive */
            int g2 = k * (3 * k + 1) / 2; /* pentagonal, k negative */
            int sign = (k % 2 == 1) ? 1 : -1;
            if (g1 <= i) total += sign * p[i - g1];
            if (g2 <= i) total += sign * p[i - g2];
            if (g1 > i && g2 > i) break;
        }
        p[i] = total;
    }
    return p[n];
}

/* Central binomial coefficient C(2n, n) = (2n)!/(n!*n!)
 * Appears in: Catalan, Erdos, random walk return, Wallis product.
 * Reference: Concrete Mathematics */
long long central_binomial(int n) {
    return combination(2 * n, n);
}

/* Wedderburn-Etherington numbers: # of full binary trees with n leaves
 * (unordered). Recurrence: w(1)=1,
 * w(n) = (1/2) * Σ_{i=1}^{n-1} w(i)*w(n-i)  for n odd
 *      + (1/2)*(Σ_{i=1}^{n-1} w(i)*w(n-i) + w(n/2)) for n even.
 * Reference: Wedderburn (1927), Etherington (1937), OEIS A001190 */
long long wedderburn_etherington(int n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    long long w[64];
    memset(w, 0, sizeof(w));
    w[1] = 1;
    for (int i = 2; i <= n && i < 64; i++) {
        long long sum = 0;
        for (int j = 1; j <= i - 1; j++)
            sum += w[j] * w[i - j];
        if (i % 2 == 0)
            w[i] = (sum + w[i / 2]) / 2;
        else
            w[i] = sum / 2;
    }
    return w[n];
}
