#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <stdbool.h>

#define MAX_GRAMMAR_PRODS 128
#define MAX_PROD_LEN     16
#define MAX_NONTERMS     26
#define MAX_TERMS        26

/* Terminals are represented as chars 'a'..'z'.
 * Nonterminals are represented as uppercase 'A'..'Z'.
 * Epsilon is represented as '\0'. */

typedef struct {
    int  lhs;              /* nonterminal (index 0..25 for A..Z) */
    int  rhs[MAX_PROD_LEN]; /* sequence of symbols; '\0' for epsilon */
    int  rhs_len;
} Production;

typedef struct {
    int  num_productions;
    Production prods[MAX_GRAMMAR_PRODS];
    int  start_symbol;  /* index 0..25 */
    bool is_cnf;        /* true after conversion to CNF */
} Grammar;

/* Create an empty grammar. Start symbol index (0..25 for A..Z). */
Grammar grammar_create(int start_symbol);

/* Add a production A -> rhs_string (null-terminated, '\0' = epsilon). */
void grammar_add_production(Grammar *g, int lhs, const char *rhs);

/* Convert grammar to Chomsky Normal Form (modifies in place). */
void grammar_to_cnf(Grammar *g);

/* CYK algorithm: test membership of `input` (length `len`) in L(G).
 * G must be in CNF. Returns true if input is in the language. */
bool grammar_cyk(const Grammar *g, const char *input, int len);

/* Generate all strings in L(G) up to given max_length.
 * Calls `callback` for each string found. */
void grammar_generate(const Grammar *g, int max_length,
                      void (*callback)(const char *str, void *ctx), void *ctx);

/* Print grammar for debugging. */
void grammar_print(const Grammar *g);

#endif /* GRAMMAR_H */
