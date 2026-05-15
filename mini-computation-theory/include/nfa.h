#ifndef NFA_H
#define NFA_H

#include <stdbool.h>
#include "dfa.h"

#define MAX_NFA_STATES 64
#define MAX_NFA_SYMBOLS 26

typedef struct {
    int num_states;
    int num_symbols;
    int start_state;
    int accept_state;  /* single accept for Thompson NFA; -1 means uses accept[] */
    bool accept[MAX_NFA_STATES];
    bool used[MAX_NFA_STATES];
    /* transitions[state][symbol] is a bitset of target states */
    unsigned long long transitions[MAX_NFA_STATES][MAX_NFA_SYMBOLS];
    /* epsilon transitions per state, as bitsets */
    unsigned long long epsilon[MAX_NFA_STATES];
} NFA;

/* Create an empty NFA. */
NFA nfa_create(int num_symbols);

/* Add a new state. Returns state index. */
int  nfa_add_state(NFA *nfa);

/* Set the start state. */
void nfa_set_start(NFA *nfa, int state);

/* Set a state as accepting. */
void nfa_set_accept(NFA *nfa, int state, bool acc);

/* Set the single accept state (for Thompson NFA). */
void nfa_set_accept_state(NFA *nfa, int state);

/* Add a normal transition: from -> on symbol -> to */
void nfa_add_transition(NFA *nfa, int from, int symbol, int to);

/* Add an epsilon transition: from -> epsilon -> to */
void nfa_add_epsilon(NFA *nfa, int from, int to);

/* Return the epsilon-closure of a set of states (bitset). */
unsigned long long nfa_epsilon_closure(const NFA *nfa, unsigned long long states);

/* Simulate NFA on input of length len (subset construction at runtime). */
bool nfa_simulate(const NFA *nfa, const int *input, int len);

/* Convert NFA to equivalent DFA via subset construction. */
DFA nfa_to_dfa(const NFA *nfa);

/* Thompson construction: union of two NFAs (modifies a, b are consumed). */
NFA nfa_thompson_union(NFA *a, NFA *b);

/* Thompson construction: concatenation a b */
NFA nfa_thompson_concat(NFA *a, NFA *b);

/* Thompson construction: Kleene star of a */
NFA nfa_thompson_star(NFA *a);

/* Create NFA for a single-symbol literal. */
NFA nfa_from_symbol(int num_symbols, int symbol);

/* Print NFA for debugging. */
void nfa_print(const NFA *nfa);

#endif /* NFA_H */
