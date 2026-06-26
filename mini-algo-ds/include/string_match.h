/**
 * String Matching Algorithms - L5: Algorithms/Methods
 *
 * Knuth-Morris-Pratt (KMP): O(n+m) worst-case, uses failure function
 * Rabin-Karp: O(n+m) expected, uses rolling hash
 * Boyer-Moore-Horspool: O(n/m) average, uses bad character shift
 *
 * MIT 6.006 Lecture 8: Hashing / 6.046J Lecture 5: String Matching
 * CLRS Chapter 32: String Matching
 */

#ifndef STRING_MATCH_H
#define STRING_MATCH_H

/* KMP: O(n+m) with failure function automaton */
void kmp_build_failure(const char* pattern, int m, int* failure);
int  kmp_search(const char* text, const char* pattern);
int  kmp_search_all(const char* text, const char* pattern, int* positions, int max_matches);

/* Rabin-Karp: Rolling hash with d=256, q=large prime */
int  rabin_karp_search(const char* text, const char* pattern);
int  rabin_karp_search_all(const char* text, const char* pattern, int* positions, int max_matches);

/* Boyer-Moore-Horspool: Simplified BM with bad character rule */
int  bmh_search(const char* text, const char* pattern);
int  bmh_search_all(const char* text, const char* pattern, int* positions, int max_matches);

/* Utilities */
int  z_algorithm(const char* str, int* z);
int  longest_palindrome_substring(const char* str, char* result, int bufsize);

#endif
