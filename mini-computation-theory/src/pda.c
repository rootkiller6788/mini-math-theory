#include "pda.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ---------------------------------------------------------------------- */
/* Create                                                                 */
/* ---------------------------------------------------------------------- */

PDA pda_create(int num_symbols) {
    PDA pda;
    pda.num_states = 0;
    pda.num_symbols = num_symbols;
    pda.start_state = 0;
    pda.num_trans = 0;
    memset(pda.accept, 0, sizeof(pda.accept));
    memset(pda.used, 0, sizeof(pda.used));
    return pda;
}

int pda_add_state(PDA *pda) {
    for (int i = 0; i < MAX_PDA_STATES; i++) {
        if (!pda->used[i]) {
            pda->used[i] = true;
            pda->accept[i] = false;
            if (i >= pda->num_states) pda->num_states = i + 1;
            return i;
        }
    }
    return -1;
}

void pda_set_start(PDA *pda, int state) {
    pda->start_state = state;
}

void pda_set_accept(PDA *pda, int state, bool acc) {
    if (state >= 0 && state < MAX_PDA_STATES)
        pda->accept[state] = acc;
}

void pda_add_transition(PDA *pda, int from, int input_char,
                        int pop_symbol, int to, const char *push_str) {
    if (pda->num_trans >= MAX_PDA_TRANS) return;
    PDATransition *t = &pda->trans[pda->num_trans++];
    t->from_state = from;
    t->input_char = input_char;
    t->pop_symbol = pop_symbol;
    t->to_state = to;
    if (push_str == NULL || push_str[0] == '\0' || push_str[0] == '\0') {
        t->push_len = 0;
    } else {
        t->push_len = (int)strlen(push_str);
        if (t->push_len > 8) t->push_len = 8;
        for (int i = 0; i < t->push_len; i++) {
            t->push_symbols[i] = push_str[i];
        }
    }
}

/* ---------------------------------------------------------------------- */
/* Simulate                                                               */
/* ---------------------------------------------------------------------- */

typedef struct {
    int state;
    int stack[MAX_STACK_SIZE];
    int top;        /* index of top element (-1 = empty) */
    int input_pos;
} PDAConfig;

static int stack_top(const PDAConfig *c) {
    if (c->top < 0) return PDA_EPSILON;
    return c->stack[c->top];
}

static void stack_push(PDAConfig *c, const int *symbols, int len) {
    for (int i = len - 1; i >= 0; i--) {
        if (c->top + 1 < MAX_STACK_SIZE) {
            c->stack[++c->top] = symbols[i];
        }
    }
}

static int stack_pop(PDAConfig *c) {
    if (c->top < 0) return PDA_EPSILON;
    return c->stack[c->top--];
}

bool pda_simulate(const PDA *pda, const int *input, int len, int max_steps) {
    /* DFS with explicit stack of configurations */
    PDAConfig init;
    init.state = pda->start_state;
    init.top = -1;
    init.input_pos = 0;
    /* Push initial stack symbol '$' */
    init.stack[++init.top] = '$';

    /* Config stack for DFS */
    PDAConfig cfg_stack[1024];
    int cfg_top = 0;
    cfg_stack[cfg_top++] = init;

    int steps = 0;

    while (cfg_top > 0) {
        if (max_steps > 0 && steps >= max_steps) return false;
        steps++;

        PDAConfig cur = cfg_stack[--cfg_top];

        /* Check if we consumed all input and are in accept state */
        if (cur.input_pos >= len && pda->accept[cur.state]) {
            return true;
        }

        int sym = (cur.input_pos < len) ? input[cur.input_pos] : PDA_EPSILON;

        /* Try all transitions from this state */
        for (int i = pda->num_trans - 1; i >= 0; i--) {
            const PDATransition *t = &pda->trans[i];
            if (t->from_state != cur.state) continue;

            /* Match input */
            if (t->input_char != PDA_EPSILON && t->input_char != sym) continue;
            if (t->input_char == PDA_EPSILON && sym != PDA_EPSILON) {
                /* Can take epsilon transition even with remaining input */
            }

            /* Match pop */
            int top_sym = stack_top(&cur);
            if (t->pop_symbol != PDA_EPSILON && t->pop_symbol != top_sym) continue;

            /* Create new configuration */
            PDAConfig next = cur;
            /* Pop */
            if (t->pop_symbol != PDA_EPSILON) {
                stack_pop(&next);
            }
            /* Push */
            if (t->push_len > 0) {
                stack_push(&next, t->push_symbols, t->push_len);
            }
            /* Advance input if this was a non-epsilon transition */
            if (t->input_char != PDA_EPSILON) {
                next.input_pos++;
            }
            next.state = t->to_state;

            /* Push onto DFS stack */
            if (cfg_top < 1024) {
                cfg_stack[cfg_top++] = next;
            }
        }
    }

    return false;
}

bool pda_accepts(const PDA *pda, const char *input, int max_steps) {
    int symbols[256];
    int len = (int)strlen(input);
    for (int i = 0; i < len && i < 256; i++) {
        symbols[i] = (int)(unsigned char)input[i];
    }
    return pda_simulate(pda, symbols, len, max_steps);
}

/* ---------------------------------------------------------------------- */
/* Print                                                                  */
/* ---------------------------------------------------------------------- */

void pda_print(const PDA *pda) {
    printf("PDA: %d states, %d symbols, start=%d, %d transitions\n",
           pda->num_states, pda->num_symbols, pda->start_state, pda->num_trans);
    printf("Accept: ");
    for (int i = 0; i < pda->num_states; i++)
        if (pda->accept[i]) printf("%d ", i);
    printf("\nTransitions:\n");
    for (int i = 0; i < pda->num_trans; i++) {
        const PDATransition *t = &pda->trans[i];
        printf("  %d --%s/%s--> %d, push=",
               t->from_state,
               t->input_char ? (char[]){t->input_char, 0} : "eps",
               t->pop_symbol ? (char[]){t->pop_symbol, 0} : "eps",
               t->to_state);
        for (int j = 0; j < t->push_len; j++)
            printf("%c", t->push_symbols[j]);
        if (t->push_len == 0) printf("ε");
        printf("\n");
    }
}
