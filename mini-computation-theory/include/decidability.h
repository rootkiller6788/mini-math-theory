#ifndef DECIDABILITY_H
#define DECIDABILITY_H

#include <stdbool.h>

/* --- Post Correspondence Problem (L4/L5: Undecidability) --- */

/*
 * PCP: Given two lists of strings (a_1,...,a_n) and (b_1,...,b_n),
 * find a non-empty sequence of indices i_1,...,i_k such that
 * a_{i1}...a_{ik} = b_{i1}...b_{ik}.
 * This problem is undecidable (Post, 1946).
 *
 * This bounded solver searches up to max_depth indices.
 * Reference: Emil Post, "A variant of a recursively unsolvable
 * problem" (1946). Used in proving undecidability of CFG ambiguity.
 */

#define MAX_PCP_TILES    32
#define MAX_PCP_STR_LEN  64
#define MAX_PCP_SEQ      128

typedef struct {
    int  num_tiles;
    char top[MAX_PCP_TILES][MAX_PCP_STR_LEN]; /* a_i strings */
    char bot[MAX_PCP_TILES][MAX_PCP_STR_LEN]; /* b_i strings */
} PCPInstance;

/* Create a PCP instance. */
PCPInstance pcp_create(void);

/* Add a tile (a_i, b_i). Returns tile index. */
int  pcp_add_tile(PCPInstance *pcp, const char *top_str, const char *bot_str);

/* Bounded solver. Fills `solution` with tile indices, returns length.
 * Returns -1 if no solution found within max_depth. */
int  pcp_solve_bounded(const PCPInstance *pcp, int max_depth, int *solution);

/* Print PCP instance. */
void pcp_print(const PCPInstance *pcp);

/* --- Rice's Theorem (L4: Standards/Theorems) --- */

/*
 * Rice's Theorem (1953): Every non-trivial semantic property
 * of Turing-machine languages is undecidable.
 *
 * A property P is:
 * - Semantic: depends only on L(M), not M's implementation
 * - Non-trivial: ∃ M₁ with P(M₁)=true, ∃ M₂ with P(M₂)=false
 *
 * This function demonstrates the theorem by reducing the
 * Halting Problem to checking a semantic property.
 * Reference: Rice (1953), "Classes of Recursively Enumerable
 * Sets and Their Decision Problems".
 */

/* Check if a property is "semantic" (depends only on accepted language).
 * This is necessarily a heuristic since the property is given as code. */
bool rice_is_semantic_property(
    bool (*property_checker)(const char *tm_encoding),
    const char *tm1, const char *tm2);

/* --- Ackermann Function (L5: Algorithm, L8: Non-primitive-recursive) --- */

/*
 * A(m,n) where:
 *   A(0,n) = n+1
 *   A(m+1,0) = A(m,1)
 *   A(m+1,n+1) = A(m, A(m+1,n))
 *
 * The Ackermann function is total computable (always halts) but NOT
 * primitive recursive. Its growth exceeds any primitive recursive function.
 * This demonstrates the gap between total computable and primitive recursive.
 *
 * Reference: Ackermann (1928), "Zum Hilbertschen Aufbau der reellen Zahlen".
 * Proved non-primitive-recursive by Rózsa Péter (1935).
 */
unsigned long long ackermann_compute(int m, int n);

/* Check if Ackermann value would overflow unsigned long long for given (m,n). */
bool ackermann_would_overflow(int m, int n);

/* --- Busy Beaver (L8: Advanced) --- */

/*
 * BB(n): maximum number of 1's printed by any n-state, 2-symbol
 * Turing machine that halts. BB(n) grows faster than any computable
 * function and is itself non-computable (Rado, 1962).
 *
 * This function computes a lower bound using known small-n values.
 * Reference: Tibor Radó (1962), "On Non-Computable Functions".
 */
int busy_beaver_lower_bound(int n);

/* BB known values: BB(1)=1, BB(2)=4, BB(3)=6, BB(4)=13.
 * BB(5) is known to be at least 4098 (proved by Marxen & Buntrock, 1990).
 * BB(6) lower bound > 10↑↑15. */
void busy_beaver_print_known(int n);

#endif /* DECIDABILITY_H */
