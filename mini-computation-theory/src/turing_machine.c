#include "turing_machine.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ---------------------------------------------------------------------- */
/* Create                                                                 */
/* ---------------------------------------------------------------------- */

TuringMachine tm_create(int num_symbols) {
    TuringMachine tm;
    tm.num_states = 0;
    tm.num_symbols = num_symbols;
    tm.start_state = 0;
    tm.num_trans = 0;
    memset(tm.accept, 0, sizeof(tm.accept));
    memset(tm.reject, 0, sizeof(tm.reject));
    memset(tm.used, 0, sizeof(tm.used));
    return tm;
}

int tm_add_state(TuringMachine *tm) {
    for (int i = 0; i < MAX_TM_STATES; i++) {
        if (!tm->used[i]) {
            tm->used[i] = true;
            tm->accept[i] = false;
            tm->reject[i] = false;
            if (i >= tm->num_states) tm->num_states = i + 1;
            return i;
        }
    }
    return -1;
}

void tm_set_accept(TuringMachine *tm, int state, bool acc) {
    if (state >= 0 && state < MAX_TM_STATES)
        tm->accept[state] = acc;
}

void tm_set_reject(TuringMachine *tm, int state, bool rej) {
    if (state >= 0 && state < MAX_TM_STATES)
        tm->reject[state] = rej;
}

void tm_add_transition(TuringMachine *tm, int state, int read_sym,
                       int next_state, int write_sym, int dir) {
    if (tm->num_trans >= MAX_TM_TRANS) return;
    TMTransition *t = &tm->transitions[tm->num_trans++];
    t->current_state = state;
    t->read_symbol = read_sym;
    t->next_state = next_state;
    t->write_symbol = write_sym;
    t->direction = dir;
}

/* ---------------------------------------------------------------------- */
/* Simulate                                                               */
/* ---------------------------------------------------------------------- */

int tm_simulate(const TuringMachine *tm, const int *input, int len, int max_steps) {
    int tape[MAX_TAPE_LEN];
    int head = MAX_TAPE_LEN / 2;
    int state = tm->start_state;

    memset(tape, TM_BLANK, sizeof(tape));
    for (int i = 0; i < len && i < MAX_TAPE_LEN / 2; i++) {
        tape[head + i] = input[i];
    }

    int steps = 0;
    while (1) {
        if (max_steps >= 0 && steps >= max_steps) return -1;
        steps++;

        if (tm->accept[state]) return 1;
        if (tm->reject[state]) return 0;

        int sym = tape[head];
        bool found = false;
        for (int i = 0; i < tm->num_trans; i++) {
            if (tm->transitions[i].current_state == state &&
                tm->transitions[i].read_symbol == sym) {
                const TMTransition *t = &tm->transitions[i];
                tape[head] = t->write_symbol;
                state = t->next_state;
                head += t->direction;
                if (head < 0 || head >= MAX_TAPE_LEN) return -1;
                found = true;
                break;
            }
        }
        if (!found) return -1;
    }
}

int tm_simulate_str(const TuringMachine *tm, const char *input, int max_steps) {
    int symbols[256];
    int len = (int)strlen(input);
    for (int i = 0; i < len && i < 256; i++) {
        symbols[i] = (int)(unsigned char)input[i];
    }
    return tm_simulate(tm, symbols, len, max_steps);
}

/* ---------------------------------------------------------------------- */
/* Print                                                                  */
/* ---------------------------------------------------------------------- */

void tm_print(const TuringMachine *tm) {
    printf("Turing Machine: %d states, %d symbols, start=%d, %d transitions\n",
           tm->num_states, tm->num_symbols, tm->start_state, tm->num_trans);
    printf("Accept: ");
    for (int i = 0; i < tm->num_states; i++)
        if (tm->accept[i]) printf("%d ", i);
    printf("\nReject: ");
    for (int i = 0; i < tm->num_states; i++)
        if (tm->reject[i]) printf("%d ", i);
    printf("\nTransitions:\n");
    for (int i = 0; i < tm->num_trans; i++) {
        const TMTransition *t = &tm->transitions[i];
        printf("  delta(%d, %d) = (%d, %d, %c)\n",
               t->current_state, t->read_symbol,
               t->next_state, t->write_symbol,
               t->direction == TM_LEFT ? 'L' : 'R');
    }
}

/* ---------------------------------------------------------------------- */
/* Built-in demo TMs                                                      */
/* ---------------------------------------------------------------------- */

/* Recognizes a^n b^n (n >= 1).
 * Strategy: cross off one 'a' on the left, one 'b' on the right, repeat.
 * When all a's and b's are crossed off and only X/Y remain, accept.
 * Tape symbols: blank=0, a=1, b=2, X=3, Y=4 */
TuringMachine tm_create_anbn(void) {
    TuringMachine tm = tm_create(5);

    int q0 = tm_add_state(&tm);   /* find first a */
    int q1 = tm_add_state(&tm);   /* scan right for b */
    int q2 = tm_add_state(&tm);   /* scan left for next a */
    int q3 = tm_add_state(&tm);   /* accept */
    int q4 = tm_add_state(&tm);   /* reject */
    tm.start_state = q0;
    tm_set_accept(&tm, q3, true);
    tm_set_reject(&tm, q4, true);

    /* q0: at left side, find first uncrossed a */
    tm_add_transition(&tm, q0, 1, q1, 3, TM_RIGHT);  /* a→X, go find b */
    tm_add_transition(&tm, q0, 4, q0, 4, TM_RIGHT);  /* Y: skip (crossed b's we passed on left) */
    tm_add_transition(&tm, q0, 3, q0, 3, TM_RIGHT);  /* X: skip crossed a'f */
    tm_add_transition(&tm, q0, 0, q3, 0, TM_RIGHT);  /* blank: all matched, accept */
    tm_add_transition(&tm, q0, 2, q4, 2, TM_RIGHT);  /* b before a → reject */

    /* q1: going right to find b */
    tm_add_transition(&tm, q1, 1, q1, 1, TM_RIGHT);  /* a: skip */
    tm_add_transition(&tm, q1, 4, q1, 4, TM_RIGHT);  /* Y: skip */
    tm_add_transition(&tm, q1, 2, q2, 4, TM_LEFT);   /* b→Y, go left */
    tm_add_transition(&tm, q1, 0, q4, 0, TM_RIGHT);  /* no b → reject */
    tm_add_transition(&tm, q1, 3, q1, 3, TM_RIGHT);  /* X: skip (shouldn't happen) */

    /* q2: going left back to find next a */
    tm_add_transition(&tm, q2, 2, q2, 2, TM_LEFT);   /* b: skip */
    tm_add_transition(&tm, q2, 1, q2, 1, TM_LEFT);   /* a: skip */
    tm_add_transition(&tm, q2, 4, q2, 4, TM_LEFT);   /* Y: skip */
    tm_add_transition(&tm, q2, 3, q0, 3, TM_RIGHT);  /* X: found crossed a, resume from q0 */

    return tm;
}

/* Recognizes w#w^R (palindrome with center marker).
 * Strategy: cross off leftmost and rightmost matching symbols one pair at a time.
 * Uses separate states for "go to end" and "check match" to avoid transition conflicts.
 * Tape symbols: blank=0, a=1, b=2, #=3, X=4 */
TuringMachine tm_create_wwr(void) {
    TuringMachine tm = tm_create(5);

    int q0  = tm_add_state(&tm);   /* start: find leftmost uncrossed symbol */
    int q1a = tm_add_state(&tm);   /* saw 'a', go right to #, then to end */
    int q1b = tm_add_state(&tm);   /* saw 'b', go right to #, then to end */
    int q2a = tm_add_state(&tm);   /* past # for 'a', go right to blank */
    int q2b = tm_add_state(&tm);   /* past # for 'b', go right to blank */
    int q3a = tm_add_state(&tm);   /* at right end, check for 'a' */
    int q3b = tm_add_state(&tm);   /* at right end, check for 'b' */
    int q4  = tm_add_state(&tm);   /* matched, going left to # */
    int q5  = tm_add_state(&tm);   /* going left from # to start marker */
    int q10 = tm_add_state(&tm);   /* verify right side all X */
    int qrej = tm_add_state(&tm);  /* reject */
    int qacc = tm_add_state(&tm);  /* accept */

    tm.start_state = q0;
    tm_set_accept(&tm, qacc, true);
    tm_set_reject(&tm, qrej, true);

    /* q0: find leftmost uncrossed symbol */
    tm_add_transition(&tm, q0, 1, q1a, 4, TM_RIGHT);    /* a → X */
    tm_add_transition(&tm, q0, 2, q1b, 4, TM_RIGHT);    /* b → X */
    tm_add_transition(&tm, q0, 3, q10, 3, TM_RIGHT);    /* # → check right side */
    tm_add_transition(&tm, q0, 4, q0,  4, TM_RIGHT);    /* X → skip */
    tm_add_transition(&tm, q0, 0, qrej,0, TM_RIGHT);    /* blank → reject */

    /* q1a: saw 'a', go right to find # */
    tm_add_transition(&tm, q1a, 1, q1a, 1, TM_RIGHT);
    tm_add_transition(&tm, q1a, 2, q1a, 2, TM_RIGHT);
    tm_add_transition(&tm, q1a, 4, q1a, 4, TM_RIGHT);
    tm_add_transition(&tm, q1a, 3, q2a, 3, TM_RIGHT);   /* # → go to right end */

    /* q1b: saw 'b', go right to find # */
    tm_add_transition(&tm, q1b, 1, q1b, 1, TM_RIGHT);
    tm_add_transition(&tm, q1b, 2, q1b, 2, TM_RIGHT);
    tm_add_transition(&tm, q1b, 4, q1b, 4, TM_RIGHT);
    tm_add_transition(&tm, q1b, 3, q2b, 3, TM_RIGHT);   /* # → go to right end */

    /* q2a: past # (for 'a'), go right to find blank */
    tm_add_transition(&tm, q2a, 1, q2a, 1, TM_RIGHT);
    tm_add_transition(&tm, q2a, 2, q2a, 2, TM_RIGHT);
    tm_add_transition(&tm, q2a, 4, q2a, 4, TM_RIGHT);
    tm_add_transition(&tm, q2a, 0, q3a, 0, TM_LEFT);    /* blank → back up and check */

    /* q2b: past # (for 'b'), go right to find blank */
    tm_add_transition(&tm, q2b, 1, q2b, 1, TM_RIGHT);
    tm_add_transition(&tm, q2b, 2, q2b, 2, TM_RIGHT);
    tm_add_transition(&tm, q2b, 4, q2b, 4, TM_RIGHT);
    tm_add_transition(&tm, q2b, 0, q3b, 0, TM_LEFT);    /* blank → back up and check */

    /* q3a: at right end, checking for 'a'. Skip X to find rightmost uncrossed. */
    tm_add_transition(&tm, q3a, 1, q4,   4, TM_LEFT);    /* a matches → X, go to # */
    tm_add_transition(&tm, q3a, 2, qrej, 2, TM_LEFT);    /* b → reject */
    tm_add_transition(&tm, q3a, 4, q3a,  4, TM_LEFT);    /* X → skip crossed, keep looking */
    tm_add_transition(&tm, q3a, 3, qrej, 3, TM_LEFT);    /* # → reject (only left of #, shouldn't happen) */

    /* q3b: at right end, checking for 'b'. Skip X to find rightmost uncrossed. */
    tm_add_transition(&tm, q3b, 2, q4,   4, TM_LEFT);    /* b matches → X, go to # */
    tm_add_transition(&tm, q3b, 1, qrej, 1, TM_LEFT);    /* a → reject */
    tm_add_transition(&tm, q3b, 4, q3b,  4, TM_LEFT);    /* X → skip crossed, keep looking */
    tm_add_transition(&tm, q3b, 3, qrej, 3, TM_LEFT);    /* # → reject */

    /* q4: matched, going left to find # */
    tm_add_transition(&tm, q4, 1, q4, 1, TM_LEFT);
    tm_add_transition(&tm, q4, 2, q4, 2, TM_LEFT);
    tm_add_transition(&tm, q4, 4, q4, 4, TM_LEFT);
    tm_add_transition(&tm, q4, 3, q5, 3, TM_LEFT);       /* # → continue left */

    /* q5: going left from # to find start marker X */
    tm_add_transition(&tm, q5, 1, q5, 1, TM_LEFT);
    tm_add_transition(&tm, q5, 2, q5, 2, TM_LEFT);
    tm_add_transition(&tm, q5, 4, q0, 4, TM_RIGHT);      /* X → resume at q0 */
    tm_add_transition(&tm, q5, 0, qrej,0, TM_RIGHT);     /* blank → reject */

    /* q10: verify right side is all X/blank */
    tm_add_transition(&tm, q10, 4, q10,  4, TM_RIGHT);   /* X: ok */
    tm_add_transition(&tm, q10, 0, qacc, 0, TM_RIGHT);   /* blank: accept */
    tm_add_transition(&tm, q10, 1, qrej, 1, TM_RIGHT);   /* leftover a → reject */
    tm_add_transition(&tm, q10, 2, qrej, 2, TM_RIGHT);   /* leftover b → reject */

    return tm;
}

/* Binary increment. Input is a binary string. TM adds 1.
 * Tape symbols: blank=0, '0'=1, '1'=2.
 * Example: "1011" → "1100" */
TuringMachine tm_create_increment(void) {
    TuringMachine tm = tm_create(3);

    int q0 = tm_add_state(&tm);   /* go to rightmost digit */
    int q1 = tm_add_state(&tm);   /* adding with carry=1 */
    int q2 = tm_add_state(&tm);   /* done, no carry */
    int q3 = tm_add_state(&tm);   /* halt/accept */
    tm.start_state = q0;
    tm_set_accept(&tm, q3, true);

    /* q0: go right to find blank after the number */
    tm_add_transition(&tm, q0, 1, q0, 1, TM_RIGHT);  /* '0' */
    tm_add_transition(&tm, q0, 2, q0, 2, TM_RIGHT);  /* '1' */
    tm_add_transition(&tm, q0, 0, q1, 0, TM_LEFT);   /* blank → start adding with carry */

    /* q1: add 1 with carry */
    tm_add_transition(&tm, q1, 1, q2, 2, TM_LEFT);   /* 0+1=1, carry=0 */
    tm_add_transition(&tm, q1, 2, q1, 1, TM_LEFT);   /* 1+1=0, carry=1 */
    tm_add_transition(&tm, q1, 0, q3, 1, TM_RIGHT);  /* overflow → write 1 */

    /* q2: just go left (addition complete) */
    tm_add_transition(&tm, q2, 1, q2, 1, TM_LEFT);
    tm_add_transition(&tm, q2, 2, q2, 2, TM_LEFT);
    tm_add_transition(&tm, q2, 0, q3, 0, TM_RIGHT);  /* done */

    return tm;
}
