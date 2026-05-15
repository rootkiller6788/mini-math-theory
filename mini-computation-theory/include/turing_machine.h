#ifndef TURING_MACHINE_H
#define TURING_MACHINE_H

#include <stdbool.h>

#define MAX_TM_STATES  32
#define MAX_TM_SYMBOLS 16
#define MAX_TM_TRANS   256
#define MAX_TAPE_LEN   1024
#define TM_BLANK       0
#define TM_LEFT       -1
#define TM_RIGHT       1

typedef struct {
    int  current_state;
    int  read_symbol;
    int  next_state;
    int  write_symbol;
    int  direction;   /* TM_LEFT or TM_RIGHT */
} TMTransition;

typedef struct {
    int num_states;
    int num_symbols;
    int start_state;
    bool accept[MAX_TM_STATES];
    bool reject[MAX_TM_STATES];  /* explicit reject states */
    bool used[MAX_TM_STATES];
    int num_trans;
    TMTransition transitions[MAX_TM_TRANS];
} TuringMachine;

/* Create a TM with given tape alphabet size (symbols 0..num_symbols-1).
 * Symbol 0 is the blank symbol. */
TuringMachine tm_create(int num_symbols);

/* Add a state. */
int  tm_add_state(TuringMachine *tm);

/* Mark state as accepting or rejecting. */
void tm_set_accept(TuringMachine *tm, int state, bool acc);
void tm_set_reject(TuringMachine *tm, int state, bool rej);

/* Add a transition. */
void tm_add_transition(TuringMachine *tm, int state, int read_sym,
                       int next_state, int write_sym, int dir);

/* Simulate the TM on `input` string (chars mapped to symbols via 'a'+idx mapping
 * used by the demo machines). max_steps caps iterations; -1 for no limit.
 * Returns: 1 = accept, 0 = reject, -1 = did not halt within max_steps. */
int  tm_simulate(const TuringMachine *tm, const int *input, int len,
                 int max_steps);

/* Convenience: simulate using a string. */
int  tm_simulate_str(const TuringMachine *tm, const char *input, int max_steps);

/* Print TM for debugging. */
void tm_print(const TuringMachine *tm);

/* --- Built-in demo Turing Machines --- */

/* Recognizes a^n b^n (n >= 1). Alphabet: a=1, b=2, blank=0. */
TuringMachine tm_create_anbn(void);

/* Recognizes w#w^R (palindrome with center marker #).
 * Alphabet: a=1, b=2, #=3, blank=0. */
TuringMachine tm_create_wwr(void);

/* Binary increment. Alphabet: 0=1, 1=2, blank=0.
 * Computes input+1 on the tape. */
TuringMachine tm_create_increment(void);

#endif /* TURING_MACHINE_H */
