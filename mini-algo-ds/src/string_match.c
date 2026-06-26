/**
 * String Matching Algorithm Implementations
 * L5: KMP (failure function automaton), Rabin-Karp (rolling hash),
 *     Boyer-Moore-Horspool (bad character rule)
 *
 * KMP Theorem: preprocessing O(m), searching O(n), total O(n+m).
 * RK Theorem: E[hash collisions] = n/q, E[runtime] = O(n+m).
 * Z-Algorithm: O(n) pattern preprocessing.
 *
 * MIT 6.006 Lecture 8 / CLRS Chapter 32
 */

#include "string_match.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* --- Knuth-Morris-Pratt (KMP) --- */

void kmp_build_failure(const char* pattern, int m, int* failure) {
    failure[0] = 0;
    int j = 0;
    for (int i = 1; i < m; i++) {
        while (j > 0 && pattern[i] != pattern[j]) j = failure[j - 1];
        if (pattern[i] == pattern[j]) j++;
        failure[i] = j;
    }
}

int kmp_search(const char* text, const char* pattern) {
    int n = (int)strlen(text), m = (int)strlen(pattern);
    if (m == 0) return 0;
    int* failure = (int*)malloc(sizeof(int) * (size_t)m);
    kmp_build_failure(pattern, m, failure);
    int j = 0, result = -1;
    for (int i = 0; i < n; i++) {
        while (j > 0 && text[i] != pattern[j]) j = failure[j - 1];
        if (text[i] == pattern[j]) j++;
        if (j == m) { result = i - m + 1; break; }
    }
    free(failure);
    return result;
}

int kmp_search_all(const char* text, const char* pattern, int* positions, int max_matches) {
    int n = (int)strlen(text), m = (int)strlen(pattern);
    if (m == 0) return 0;
    int* failure = (int*)malloc(sizeof(int) * (size_t)m);
    kmp_build_failure(pattern, m, failure);
    int j = 0, count = 0;
    for (int i = 0; i < n && count < max_matches; i++) {
        while (j > 0 && text[i] != pattern[j]) j = failure[j - 1];
        if (text[i] == pattern[j]) j++;
        if (j == m) { positions[count++] = i - m + 1; j = failure[j - 1]; }
    }
    free(failure);
    return count;
}

/* --- Rabin-Karp --- */

#define RK_BASE 256
#define RK_PRIME 1000000007

/* Rolling hash: compute hash of str[0..len-1] */
static long long compute_hash(const char* str, int len) {
    long long h = 0;
    for (int i = 0; i < len; i++)
        h = (h * RK_BASE + (unsigned char)str[i]) % RK_PRIME;
    return h;
}

/* Compute base^(len-1) mod prime for rolling update */
static long long compute_pow(int len) {
    long long p = 1;
    for (int i = 0; i < len - 1; i++)
        p = (p * RK_BASE) % RK_PRIME;
    return p;
}

int rabin_karp_search(const char* text, const char* pattern) {
    int n = (int)strlen(text), m = (int)strlen(pattern);
    if (m == 0) return 0;
    if (m > n) return -1;
    long long ph = compute_hash(pattern, m);
    long long th = compute_hash(text, m);
    long long pow = compute_pow(m);
    for (int i = 0; i <= n - m; i++) {
        if (th == ph) {
            bool match = true;
            for (int j = 0; j < m; j++)
                if (text[i + j] != pattern[j]) { match = false; break; }
            if (match) return i;
        }
        if (i < n - m) {
            th = (th - (unsigned char)text[i] * pow % RK_PRIME + RK_PRIME) % RK_PRIME;
            th = (th * RK_BASE + (unsigned char)text[i + m]) % RK_PRIME;
        }
    }
    return -1;
}

int rabin_karp_search_all(const char* text, const char* pattern, int* positions, int max_matches) {
    int n = (int)strlen(text), m = (int)strlen(pattern);
    if (m == 0) return 0;
    if (m > n) return 0;
    long long ph = compute_hash(pattern, m);
    long long th = compute_hash(text, m);
    long long pow = compute_pow(m);
    int count = 0;
    for (int i = 0; i <= n - m && count < max_matches; i++) {
        if (th == ph) {
            bool match = true;
            for (int j = 0; j < m; j++)
                if (text[i + j] != pattern[j]) { match = false; break; }
            if (match) positions[count++] = i;
        }
        if (i < n - m) {
            th = (th - (unsigned char)text[i] * pow % RK_PRIME + RK_PRIME) % RK_PRIME;
            th = (th * RK_BASE + (unsigned char)text[i + m]) % RK_PRIME;
        }
    }
    return count;
}

/* --- Boyer-Moore-Horspool (simplified BM with bad character rule) --- */

int bmh_search(const char* text, const char* pattern) {
    int n = (int)strlen(text), m = (int)strlen(pattern);
    if (m == 0) return 0;
    if (m > n) return -1;
    /* Bad character table: last occurrence of each byte */
    int bad_char[256];
    for (int i = 0; i < 256; i++) bad_char[i] = m;
    for (int i = 0; i < m - 1; i++) bad_char[(unsigned char)pattern[i]] = m - 1 - i;
    int i = 0;
    while (i <= n - m) {
        int j = m - 1;
        while (j >= 0 && pattern[j] == text[i + j]) j--;
        if (j < 0) return i;
        i += bad_char[(unsigned char)text[i + m - 1]];
        /* Ensure positive shift */
        if (i < 0) i = 0;
    }
    return -1;
}

int bmh_search_all(const char* text, const char* pattern, int* positions, int max_matches) {
    int n = (int)strlen(text), m = (int)strlen(pattern);
    if (m == 0 || m > n) return 0;
    int bad_char[256];
    for (int i = 0; i < 256; i++) bad_char[i] = m;
    for (int i = 0; i < m - 1; i++) bad_char[(unsigned char)pattern[i]] = m - 1 - i;
    int count = 0, i = 0;
    while (i <= n - m && count < max_matches) {
        int j = m - 1;
        while (j >= 0 && pattern[j] == text[i + j]) j--;
        if (j < 0) {
            positions[count++] = i;
            i++;  /* simple shift for overlapping matches */
        } else {
            i += bad_char[(unsigned char)text[i + m - 1]];
        }
    }
    return count;
}

/* --- Z-Algorithm --- */
/* Z[i] = longest substring starting at i that matches prefix */
int z_algorithm(const char* str, int* z) {
    int n = (int)strlen(str);
    if (n == 0) return 0;
    z[0] = 0;
    int l = 0, r = 0;
    for (int i = 1; i < n; i++) {
        if (i <= r) {
            int k = i - l;
            if (z[k] < r - i + 1) z[i] = z[k];
            else { l = i; while (r < n && str[r - l] == str[r]) r++; z[i] = r - l; r--; }
        } else {
            l = r = i;
            while (r < n && str[r - l] == str[r]) r++;
            z[i] = r - l;
            r--;
        }
    }
    return n;
}

/* --- Longest Palindromic Substring (Manacher-style) --- */
int longest_palindrome_substring(const char* str, char* result, int bufsize) {
    int n = (int)strlen(str);
    if (n == 0) { result[0] = '\0'; return 0; }
    int start = 0, max_len = 1;
    for (int center = 0; center < n; center++) {
        /* Odd length palindromes */
        int l = center, r = center;
        while (l >= 0 && r < n && str[l] == str[r]) { l--; r++; }
        int len = r - l - 1;
        if (len > max_len) { max_len = len; start = l + 1; }
        /* Even length palindromes */
        l = center; r = center + 1;
        while (l >= 0 && r < n && str[l] == str[r]) { l--; r++; }
        len = r - l - 1;
        if (len > max_len) { max_len = len; start = l + 1; }
    }
    int copy_len = max_len < bufsize - 1 ? max_len : bufsize - 1;
    for (int i = 0; i < copy_len; i++) result[i] = str[start + i];
    result[copy_len] = '\0';
    return max_len;
}
