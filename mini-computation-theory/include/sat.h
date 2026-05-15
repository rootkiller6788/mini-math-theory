#ifndef SAT_H
#define SAT_H

#include <stdbool.h>

#define MAX_SAT_VARS    32
#define MAX_SAT_CLAUSES 128
#define MAX_CLAUSE_LEN  16

typedef struct {
    int literals[MAX_CLAUSE_LEN];
    int len;
} Clause;

typedef struct {
    int  num_vars;
    int  num_clauses;
    Clause clauses[MAX_SAT_CLAUSES];
} SAT;

/* Create a SAT instance with n variables. */
SAT sat_create(int num_vars);

/* Add a new empty clause. Returns index. */
int  sat_add_clause(SAT *sat);

/* Add a literal to clause at index `clause_idx`.
 * Positive integer = variable, negative = negated variable. */
void sat_add_literal(SAT *sat, int clause_idx, int literal);

/* Solve using DPLL algorithm. Puts assignment in `assignment` (1-indexed, 0=unset).
 * Returns true if satisfiable. assignment array must be size num_vars+1. */
bool sat_solve(const SAT *sat, int *assignment);

/* Print the SAT formula. */
void sat_print(const SAT *sat);

#endif /* SAT_H */
