#ifndef DFA_H
#define DFA_H

#include <stdbool.h>

#define MAX_DFA_STATES 64
#define MAX_DFA_SYMBOLS 26
#define DFA_DEAD_STATE -1

typedef struct {
    int num_states;
    int num_symbols;
    int transitions[MAX_DFA_STATES][MAX_DFA_SYMBOLS];
    int start_state;
    bool accept[MAX_DFA_STATES];
    bool used[MAX_DFA_STATES];
} DFA;

/* Create an empty DFA with given alphabet size. Returns the DFA on stack. */
DFA dfa_create(int num_symbols);

/* Add a state to the DFA. Returns state index. */
int  dfa_add_state(DFA *dfa);

/* Mark a state as accepting. */
void dfa_set_accept(DFA *dfa, int state, bool acc);

/* Set the start state. */
void dfa_set_start(DFA *dfa, int state);

/* Add a transition: from state `s` on symbol `c` go to `next`. */
void dfa_add_transition(DFA *dfa, int s, int c, int next);

/* Simulate the DFA on input string `input` of length `len`. Returns true if accepted. */
bool dfa_simulate(const DFA *dfa, const int *input, int len);

/* Minimize the DFA using partition refinement. Modifies dfa in place. */
void dfa_minimize(DFA *dfa);

/* Return a new DFA that is the complement of `dfa`. */
DFA dfa_complement(const DFA *dfa);

/* Return a new DFA that is the union of L(a) and L(b). */
DFA dfa_union(const DFA *a, const DFA *b);

/* Return a new DFA that is the intersection of L(a) and L(b). */
DFA dfa_intersection(const DFA *a, const DFA *b);

/* Print the DFA transition table for debugging. */
void dfa_print(const DFA *dfa);

#endif /* DFA_H */
