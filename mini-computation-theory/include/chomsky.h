#ifndef CHOMSKY_H
#define CHOMSKY_H

#include <stdbool.h>
#include "grammar.h"

/* Chomsky hierarchy types (L4: Standards/Theorems) */
typedef enum {
    CHOMSKY_TYPE_0 = 0,  /* Unrestricted (recursively enumerable) */
    CHOMSKY_TYPE_1 = 1,  /* Context-sensitive */
    CHOMSKY_TYPE_2 = 2,  /* Context-free */
    CHOMSKY_TYPE_3 = 3,  /* Regular */
    CHOMSKY_UNKNOWN = -1
} ChomskyType;

/* --- Chomsky Hierarchy Classification (L4) --- */

/*
 * Classify the grammar according to Chomsky hierarchy.
 * Checks production forms to determine the minimal type.
 *
 * Type 3 (Regular):  A → aB | A → a | A → ε
 * Type 2 (CF):       A → α  for any α ∈ (N ∪ T)*
 * Type 1 (CS):       αAβ → αγβ  with |γ| ≥ 1
 * Type 0 (Unrestricted): arbitrary
 *
 * Reference: Chomsky (1956), "Three models for the description
 * of language"; Chomsky (1959), "On certain formal properties
 * of grammars".
 */
ChomskyType chomsky_classify_grammar(const Grammar *g);

/* Verify that a grammar is in Chomsky Normal Form.
 * Returns true and fills `reason` on failure. */
bool chomsky_verify_cnf(const Grammar *g, char *reason, int reason_sz);

/* Verify that a grammar is in Greibach Normal Form (L4).
 * GNF: A → aα where a ∈ T, α ∈ N*.
 * Reference: Greibach (1965), "A New Normal-Form Theorem
 * for Context-Free Phrase Structure Grammars". */
bool chomsky_verify_gnf(const Grammar *g, char *reason, int reason_sz);

/* Detect if a CFG generates a regular language (L8 Advanced).
 * Uses the self-embedding criterion. */
bool chomsky_is_regular_language(const Grammar *g);

/* --- L-Systems (Lindenmayer Systems) — L7 Application --- */

#define MAX_LSYM_PRODS  64
#define MAX_LSYM_LEN     256

typedef struct {
    int  symbol;
    int  replacement[MAX_LSYM_LEN];
    int  repl_len;
} LSystemProd;

typedef struct {
    int  num_prods;
    LSystemProd prods[MAX_LSYM_PRODS];
    int  axiom[MAX_LSYM_LEN];
    int  axiom_len;
    int  num_symbols;
} LSystem;

/* Create an L-System with given alphabet size. */
LSystem lsystem_create(int num_symbols);

/* Set the axiom. */
void lsystem_set_axiom(LSystem *ls, const int *axiom, int len);

/* Add a production. */
void lsystem_add_production(LSystem *ls, int symbol,
                            const int *repl, int len);

/* One iteration of parallel rewriting.
 * Returns length of result. If out=NULL, returns needed size. */
int  lsystem_iterate(const LSystem *ls, const int *current, int cur_len,
                     int *out, int max_out);

/* Perform n iterations from axiom.
 * Returns static buffer (not thread-safe). Set *out_len. */
const int* lsystem_generate(LSystem *ls, int n, int *out_len, int max_total);

/* --- CFL Pumping Lemma (L4: Standards/Theorems) --- */

typedef struct {
    int u_len, v_len, w_len, x_len, y_len;
    int pumping_len;
} CFLPumpDecomp;

bool cfl_pumping_decompose(const Grammar *g, const char *input, int len,
                           CFLPumpDecomp *decomp);

bool cfl_pumping_verify(const Grammar *g, const char *input, int len,
                        const CFLPumpDecomp *decomp, int pump_count);

#endif /* CHOMSKY_H */
