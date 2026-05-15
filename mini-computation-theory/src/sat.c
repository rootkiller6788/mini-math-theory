#include "sat.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ---------------------------------------------------------------------- */
/* Create                                                                 */
/* ---------------------------------------------------------------------- */

SAT sat_create(int num_vars) {
    SAT sat;
    sat.num_vars = num_vars;
    sat.num_clauses = 0;
    return sat;
}

int sat_add_clause(SAT *sat) {
    if (sat->num_clauses >= MAX_SAT_CLAUSES) return -1;
    sat->clauses[sat->num_clauses].len = 0;
    return sat->num_clauses++;
}

void sat_add_literal(SAT *sat, int clause_idx, int literal) {
    if (clause_idx < 0 || clause_idx >= sat->num_clauses) return;
    Clause *c = &sat->clauses[clause_idx];
    if (c->len >= MAX_CLAUSE_LEN) return;
    c->literals[c->len++] = literal;
}

/* ---------------------------------------------------------------------- */
/* DPLL Algorithm                                                         */
/* ---------------------------------------------------------------------- */

typedef struct {
    int assignment[MAX_SAT_VARS + 1];  /* 0=unset, 1=true, -1=false */
    int num_vars;
} DPLLState;

/* Check if a literal is true under current assignment */
static bool literal_true(int lit, const int *assign) {
    int var = (lit > 0) ? lit : -lit;
    int val = assign[var];
    if (val == 0) return false;
    return (lit > 0) ? (val == 1) : (val == -1);
}

/* Check if a literal is false under current assignment */
static bool literal_false(int lit, const int *assign) {
    int var = (lit > 0) ? lit : -lit;
    int val = assign[var];
    if (val == 0) return false;
    return (lit > 0) ? (val == -1) : (val == 1);
}

/* Check if literal is unassigned */
static bool literal_unset(int lit, const int *assign) {
    int var = (lit > 0) ? lit : -lit;
    return assign[var] == 0;
}

/* Clause status: 0=unknown, 1=satisfied, -1=conflict */
static int clause_status(const Clause *c, const int *assign) {
    bool all_false = true;
    for (int i = 0; i < c->len; i++) {
        if (literal_true(c->literals[i], assign)) return 1;
        if (!literal_false(c->literals[i], assign)) all_false = false;
    }
    if (all_false) return -1;
    return 0;
}

/* Find a unit clause: exactly one unset literal, all others false */
static int find_unit_clause(const Clause *clauses, int n, const int *assign) {
    for (int i = 0; i < n; i++) {
        int unset_count = 0;
        int unset_lit = 0;
        bool satisfied = false;
        for (int j = 0; j < clauses[i].len; j++) {
            int lit = clauses[i].literals[j];
            if (literal_true(lit, assign)) { satisfied = true; break; }
            if (literal_unset(lit, assign)) {
                unset_count++;
                unset_lit = lit;
            }
        }
        if (!satisfied && unset_count == 1) return unset_lit;
    }
    return 0;
}

/* Find a pure literal */
static int find_pure_literal(const Clause *clauses, int n, int num_vars, const int *assign) {
    bool pos_occurs[MAX_SAT_VARS + 1] = {false};
    bool neg_occurs[MAX_SAT_VARS + 1] = {false};

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < clauses[i].len; j++) {
            int lit = clauses[i].literals[j];
            int var = (lit > 0) ? lit : -lit;
            if (assign[var] != 0) continue;
            if (lit > 0) pos_occurs[var] = true;
            else neg_occurs[var] = true;
        }
    }

    for (int v = 1; v <= num_vars; v++) {
        if (assign[v] != 0) continue;
        if (pos_occurs[v] && !neg_occurs[v]) return v;   /* pure positive */
        if (!pos_occurs[v] && neg_occurs[v]) return -v;   /* pure negative */
    }
    return 0;
}

/* Find an unassigned variable (for branching) */
static int find_unassigned_var(const int *assign, int num_vars) {
    for (int v = 1; v <= num_vars; v++)
        if (assign[v] == 0) return v;
    return 0;
}

static bool dpll_rec(const Clause *clauses, int n, DPLLState *state);

bool dpll_rec(const Clause *clauses, int n, DPLLState *state) {
    /* Check all clauses */
    bool all_sat = true;
    for (int i = 0; i < n; i++) {
        int status = clause_status(&clauses[i], state->assignment);
        if (status == -1) return false; /* conflict */
        if (status != 1) all_sat = false;
    }
    if (all_sat) return true;

    /* Unit propagation */
    int unit;
    while ((unit = find_unit_clause(clauses, n, state->assignment)) != 0) {
        int var = (unit > 0) ? unit : -unit;
        state->assignment[var] = (unit > 0) ? 1 : -1;
    }

    /* Check again after unit propagation */
    all_sat = true;
    for (int i = 0; i < n; i++) {
        int status = clause_status(&clauses[i], state->assignment);
        if (status == -1) return false;
        if (status != 1) all_sat = false;
    }
    if (all_sat) return true;

    /* Pure literal elimination */
    int pure;
    while ((pure = find_pure_literal(clauses, n, state->num_vars, state->assignment)) != 0) {
        int var = (pure > 0) ? pure : -pure;
        state->assignment[var] = (pure > 0) ? 1 : -1;
    }

    /* Check after pure literal elimination */
    all_sat = true;
    for (int i = 0; i < n; i++) {
        int status = clause_status(&clauses[i], state->assignment);
        if (status == -1) return false;
        if (status != 1) all_sat = false;
    }
    if (all_sat) return true;

    /* Branch */
    int var = find_unassigned_var(state->assignment, state->num_vars);
    if (var == 0) {
        /* All variables assigned but not all clauses satisfied → UNSAT */
        return false;
    }

    /* Save state for backtracking */
    int saved[MAX_SAT_VARS + 1];
    memcpy(saved, state->assignment, sizeof(saved));

    /* Try true */
    state->assignment[var] = 1;
    if (dpll_rec(clauses, n, state)) return true;

    /* Backtrack and try false */
    memcpy(state->assignment, saved, sizeof(saved));
    state->assignment[var] = -1;
    if (dpll_rec(clauses, n, state)) return true;

    /* Restore */
    memcpy(state->assignment, saved, sizeof(saved));
    return false;
}

bool sat_solve(const SAT *sat, int *assignment) {
    DPLLState state;
    state.num_vars = sat->num_vars;
    for (int i = 0; i <= sat->num_vars; i++) {
        state.assignment[i] = 0;
    }

    bool result = dpll_rec(sat->clauses, sat->num_clauses, &state);
    if (assignment != NULL) {
        memcpy(assignment, state.assignment, (sat->num_vars + 1) * sizeof(int));
    }
    return result;
}

/* ---------------------------------------------------------------------- */
/* Print                                                                  */
/* ---------------------------------------------------------------------- */

void sat_print(const SAT *sat) {
    printf("SAT: %d variables, %d clauses\n", sat->num_vars, sat->num_clauses);
    for (int i = 0; i < sat->num_clauses; i++) {
        printf("  ");
        for (int j = 0; j < sat->clauses[i].len; j++) {
            int lit = sat->clauses[i].literals[j];
            if (lit < 0) printf("¬x%d ", -lit);
            else printf("x%d ", lit);
        }
        printf("\n");
    }
}
