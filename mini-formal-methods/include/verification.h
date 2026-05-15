#ifndef VERIFICATION_H
#define VERIFICATION_H

#include <stdbool.h>

#define MAX_SYM_VARS 6
#define MAX_STATES 128
#define MAX_BMC_STEPS 32

typedef struct {
    int value;
    bool is_symbolic;
    int lo, hi;
    int current;
} SymVar;

typedef struct {
    SymVar vars[MAX_SYM_VARS];
    int nvars;
    int paths_explored;
    int counterexamples;
} SymExecutor;

void sym_exec_init(SymExecutor* se);
void sym_exec_add_var(SymExecutor* se, int lo, int hi);
bool sym_exec_explore(SymExecutor* se,
                       bool (*property)(const int* values),
                       void (*print_ce)(const int* values));

typedef struct {
    int transitions[MAX_STATES][MAX_STATES];
    int nstates;
    int initial;
    bool is_deterministic;
} StateMachine;

void sm_init(StateMachine* sm, int nstates, int initial);
void sm_add_transition(StateMachine* sm, int from, int to);

typedef struct {
    const int* trace;
    int length;
} BMCResult;

bool bmc_check(StateMachine* sm,
               bool (*property)(int state),
               int bound,
               BMCResult* result);

#endif
