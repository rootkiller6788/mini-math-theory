#include "decidability.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

/* ======================================================================
 * Post Correspondence Problem (L4/L5: Undecidability)
 *
 * PCP: Given tiles (a_i, b_i), find sequence i_1..i_k such that
 * a_i1...a_ik = b_i1...b_ik. This problem is undecidable (Post, 1946).
 *
 * This bounded solver uses DFS up to max_depth.
 * Reference: Emil L. Post, "A Variant of a Recursively Unsolvable
 * Problem", Bulletin of the AMS, 1946.
 * ====================================================================== */

PCPInstance pcp_create(void) {
    PCPInstance pcp;
    pcp.num_tiles = 0;
    memset(pcp.top, 0, sizeof(pcp.top));
    memset(pcp.bot, 0, sizeof(pcp.bot));
    return pcp;
}

int pcp_add_tile(PCPInstance *pcp, const char *top_str, const char *bot_str) {
    if (pcp->num_tiles >= MAX_PCP_TILES) return -1;
    int idx = pcp->num_tiles++;
    strncpy(pcp->top[idx], top_str, MAX_PCP_STR_LEN - 1);
    pcp->top[idx][MAX_PCP_STR_LEN - 1] = '\0';
    strncpy(pcp->bot[idx], bot_str, MAX_PCP_STR_LEN - 1);
    pcp->bot[idx][MAX_PCP_STR_LEN - 1] = '\0';
    return idx;
}

/* DFS bounded search for PCP solution */
static int pcp_dfs(const PCPInstance *pcp, int depth, int max_depth,
                   const char *top_sofar, const char *bot_sofar,
                   int *solution, int sol_len) {
    int tlen = (int)strlen(top_sofar);
    int blen = (int)strlen(bot_sofar);

    /* Solution found: non-empty, equal strings */
    if (sol_len > 0 && tlen > 0 && tlen == blen) {
        if (strcmp(top_sofar, bot_sofar) == 0) return sol_len;
    }
    if (depth >= max_depth) return -1;

    for (int t = 0; t < pcp->num_tiles; t++) {
        char new_top[2048], new_bot[2048];
        snprintf(new_top, sizeof(new_top), "%s%s", top_sofar, pcp->top[t]);
        snprintf(new_bot, sizeof(new_bot), "%s%s", bot_sofar, pcp->bot[t]);

        int ntlen = (int)strlen(new_top);
        int nblen = (int)strlen(new_bot);
        int minl = (ntlen < nblen) ? ntlen : nblen;

        /* Prune: prefixes must match */
        if (strncmp(new_top, new_bot, minl) != 0) continue;

        if (sol_len < MAX_PCP_SEQ) solution[sol_len] = t;
        int sub = pcp_dfs(pcp, depth + 1, max_depth,
                          new_top, new_bot, solution, sol_len + 1);
        if (sub > 0) return sub;
        if (sub == 0) continue;
    }
    return -1;
}

int pcp_solve_bounded(const PCPInstance *pcp, int max_depth, int *solution) {
    if (max_depth <= 0) max_depth = 6;
    return pcp_dfs(pcp, 0, max_depth, "", "", solution, 0);
}

void pcp_print(const PCPInstance *pcp) {
    printf("PCP: %d tiles\n", pcp->num_tiles);
    for (int i = 0; i < pcp->num_tiles; i++)
        printf("  tile %d: [%s] / [%s]\n", i, pcp->top[i], pcp->bot[i]);
}

/* ======================================================================
 * Rice's Theorem (L4: Standards/Theorems, L8: Advanced)
 *
 * Rice (1953): Every non-trivial semantic property of RE sets
 * is undecidable.
 * ====================================================================== */

bool rice_is_semantic_property(
    bool (*property_checker)(const char *tm_encoding),
    const char *tm1, const char *tm2) {
    bool p1 = property_checker(tm1);
    bool p2 = property_checker(tm2);
    return (p1 != p2);  /* non-trivial property: one has it, one doesn't */
}

/* ======================================================================
 * Ackermann Function (L5: Algorithm, L8: Non-primitive-recursive)
 *
 * The Ackermann function A(m,n) is total computable but NOT
 * primitive recursive. Its growth exceeds any primitive recursive
 * function.
 *
 * A(0,n) = n+1
 * A(m+1,0) = A(m,1)
 * A(m+1,n+1) = A(m, A(m+1,n))
 *
 * For m=0..3, closed forms exist:
 *   A(0,n) = n+1
 *   A(1,n) = n+2
 *   A(2,n) = 2n+3
 *   A(3,n) = 2^(n+3) - 3
 *
 * For m>=4, recursion is used with a depth limit.
 *
 * Reference: Ackermann (1928), Peter (1935).
 * ====================================================================== */

/* Recursive Ackermann with depth bound */
static unsigned long long ack_rec(int m, int n, int depth, int max_depth) {
    if (depth > max_depth) return ULLONG_MAX;
    if (m == 0) return (unsigned long long)n + 1;
    if (m == 1) return (unsigned long long)n + 2;
    if (m == 2) return 2ULL * (unsigned long long)n + 3;
    if (m == 3) {
        if (n > 60) return ULLONG_MAX;
        return (1ULL << (n + 3)) - 3;
    }
    /* m >= 4: use recursive definition */
    if (n == 0) return ack_rec(m - 1, 1, depth + 1, max_depth);
    unsigned long long inner = ack_rec(m, n - 1, depth + 1, max_depth);
    if (inner == ULLONG_MAX) return ULLONG_MAX;
    return ack_rec(m - 1, (int)inner, depth + 1, max_depth);
}

unsigned long long ackermann_compute(int m, int n) {
    if (m < 0 || n < 0) return ULLONG_MAX;
    if (ackermann_would_overflow(m, n)) return ULLONG_MAX;
    return ack_rec(m, n, 0, 100000);
}

bool ackermann_would_overflow(int m, int n) {
    if (m == 0) return false;
    if (m == 1) return false;
    if (m == 2) return (unsigned long long)n > (ULLONG_MAX - 3) / 2;
    if (m == 3) return n > 60;
    if (m == 4) return n > 1;
    return true;
}

/* ======================================================================
 * Busy Beaver Function (L8: Advanced, L9: Industry Frontiers)
 *
 * Sigma(n): max 1's written by halting n-state 2-symbol TM.
 * Non-computable, grows faster than any computable function.
 * Reference: Tibor Rado, "On Non-Computable Functions", 1962.
 * ====================================================================== */

int busy_beaver_lower_bound(int n) {
    switch (n) {
        case 0:  return 0;
        case 1:  return 1;
        case 2:  return 4;
        case 3:  return 6;
        case 4:  return 13;
        case 5:  return 4098;
        case 6:  return 95;  /* > 10^^15 conservatively */
        default: return -1;
    }
}

void busy_beaver_print_known(int n) {
    printf("Busy Beaver lower bounds:\n");
    printf("  BB(0) = 0\n");
    printf("  BB(1) = 1  (exact, Rado 1962)\n");
    printf("  BB(2) = 4  (exact, Rado 1962)\n");
    printf("  BB(3) = 6  (exact, Lin & Rado 1965)\n");
    printf("  BB(4) = 13 (exact, Brady 1983)\n");
    printf("  BB(5) >= 4098 (Marxen & Buntrock 1990)\n");
    printf("  BB(6) > 10^^15 (Ligocki 2022)\n");
    if (n >= 0 && n <= 6)
        printf("  BB(%d) >= %d\n", n, busy_beaver_lower_bound(n));
    else if (n > 6)
        printf("  BB(%d): unknown, uncomputable\n", n);
}
