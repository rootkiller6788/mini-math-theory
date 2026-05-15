#ifndef PDA_H
#define PDA_H

#include <stdbool.h>

#define MAX_PDA_STATES    32
#define MAX_PDA_SYMBOLS   26
#define MAX_PDA_TRANS     256
#define MAX_STACK_SYMBOLS 26
#define MAX_STACK_SIZE    128
#define PDA_EPSILON       '\0'

typedef struct {
    int from_state;
    int input_char;      /* PDA_EPSILON for epsilon transition */
    int pop_symbol;      /* PDA_EPSILON for no pop */
    int to_state;
    int push_len;        /* number of symbols to push */
    int push_symbols[8]; /* symbols to push (first is top-of-stack after push) */
} PDATransition;

typedef struct {
    int num_states;
    int num_symbols;
    int start_state;
    int num_trans;
    PDATransition trans[MAX_PDA_TRANS];
    bool accept[MAX_PDA_STATES];
    bool used[MAX_PDA_STATES];
} PDA;

/* Create PDA with given alphabet size. */
PDA pda_create(int num_symbols);

/* Add a state. Returns index. */
int  pda_add_state(PDA *pda);

/* Set start state. */
void pda_set_start(PDA *pda, int state);

/* Mark state as accepting (for final-state acceptance). */
void pda_set_accept(PDA *pda, int state, bool acc);

/* Add a transition: from_state --input/pop--> to_state, push stack symbols.
 * push_str is a null-terminated string of symbols to push (first char = top).
 * Use PDA_EPSILON ('\0') in push_str for no push.
 * Use PDA_EPSILON for input_char or pop_symbol for epsilon. */
void pda_add_transition(PDA *pda, int from, int input_char,
                        int pop_symbol, int to, const char *push_str);

/* Simulate PDA on `input` of length `len`.
 * max_steps limits search depth (0 = unlimited).
 * Returns true if accepted by final state. */
bool pda_simulate(const PDA *pda, const int *input, int len, int max_steps);

/* Convenience: accepts given a string. */
bool pda_accepts(const PDA *pda, const char *input, int max_steps);

/* Print PDA for debugging. */
void pda_print(const PDA *pda);

#endif /* PDA_H */
