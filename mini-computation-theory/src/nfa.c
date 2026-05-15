#include "nfa.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ---------------------------------------------------------------------- */
/* Helpers: bitset operations                                             */
/* ---------------------------------------------------------------------- */

static inline void bs_set(unsigned long long *bs, int i) {
    *bs |= (1ULL << i);
}
static inline bool bs_test(unsigned long long bs, int i) {
    return (bs >> i) & 1ULL;
}
static inline void bs_clear(unsigned long long *bs, int i) {
    *bs &= ~(1ULL << i);
}

/* ---------------------------------------------------------------------- */
/* Create                                                                 */
/* ---------------------------------------------------------------------- */

NFA nfa_create(int num_symbols) {
    NFA nfa;
    nfa.num_states = 0;
    nfa.num_symbols = num_symbols;
    nfa.start_state = 0;
    nfa.accept_state = -1;
    memset(nfa.accept, 0, sizeof(nfa.accept));
    memset(nfa.used, 0, sizeof(nfa.used));
    memset(nfa.transitions, 0, sizeof(nfa.transitions));
    memset(nfa.epsilon, 0, sizeof(nfa.epsilon));
    return nfa;
}

int nfa_add_state(NFA *nfa) {
    for (int i = 0; i < MAX_NFA_STATES; i++) {
        if (!nfa->used[i]) {
            nfa->used[i] = true;
            nfa->accept[i] = false;
            memset(nfa->transitions[i], 0, sizeof(nfa->transitions[i]));
            nfa->epsilon[i] = 0;
            if (i >= nfa->num_states) {
                nfa->num_states = i + 1;
            }
            return i;
        }
    }
    return -1;
}

void nfa_set_start(NFA *nfa, int state) {
    nfa->start_state = state;
}

void nfa_set_accept(NFA *nfa, int state, bool acc) {
    if (state >= 0 && state < MAX_NFA_STATES) {
        nfa->accept[state] = acc;
    }
}

void nfa_set_accept_state(NFA *nfa, int state) {
    nfa->accept_state = state;
    nfa->accept[state] = true;
}

void nfa_add_transition(NFA *nfa, int from, int symbol, int to) {
    if (from >= 0 && from < MAX_NFA_STATES &&
        symbol >= 0 && symbol < nfa->num_symbols &&
        to >= 0 && to < MAX_NFA_STATES) {
        bs_set(&nfa->transitions[from][symbol], to);
    }
}

void nfa_add_epsilon(NFA *nfa, int from, int to) {
    if (from >= 0 && from < MAX_NFA_STATES && to >= 0 && to < MAX_NFA_STATES) {
        bs_set(&nfa->epsilon[from], to);
    }
}

/* ---------------------------------------------------------------------- */
/* Epsilon closure                                                        */
/* ---------------------------------------------------------------------- */

unsigned long long nfa_epsilon_closure(const NFA *nfa, unsigned long long states) {
    unsigned long long closure = states;
    unsigned long long stack = states;
    while (stack) {
        /* pick a bit from stack */
        int s;
        for (s = 0; s < nfa->num_states; s++) {
            if (bs_test(stack, s)) break;
        }
        bs_clear(&stack, s);
        unsigned long long eps = nfa->epsilon[s];
        unsigned long long new_states = eps & ~closure;
        closure |= new_states;
        stack |= new_states;
    }
    return closure;
}

/* ---------------------------------------------------------------------- */
/* Simulate (runtime subset construction)                                 */
/* ---------------------------------------------------------------------- */

bool nfa_simulate(const NFA *nfa, const int *input, int len) {
    unsigned long long cur = nfa_epsilon_closure(nfa, 1ULL << nfa->start_state);
    for (int i = 0; i < len; i++) {
        int sym = input[i];
        if (sym < 0 || sym >= nfa->num_symbols) return false;
        unsigned long long next = 0;
        for (int s = 0; s < nfa->num_states; s++) {
            if (bs_test(cur, s)) {
                next |= nfa->transitions[s][sym];
            }
        }
        cur = nfa_epsilon_closure(nfa, next);
        if (cur == 0) return false;
    }
    /* check for accept */
    for (int s = 0; s < nfa->num_states; s++) {
        if (bs_test(cur, s) && nfa->accept[s]) return true;
    }
    return false;
}

/* ---------------------------------------------------------------------- */
/* NFA → DFA subset construction                                          */
/* ---------------------------------------------------------------------- */

DFA nfa_to_dfa(const NFA *nfa) {
    DFA dfa = dfa_create(nfa->num_symbols);

    /* We'll store subsets as bitsets. map: bitset → dfa state index */
    unsigned long long subsets[256];
    int dfa_state_idx[256]; /* maps subset index -> dfa state number */
    int num_subsets = 0;

    /* start subset */
    unsigned long long start_set = nfa_epsilon_closure(nfa, 1ULL << nfa->start_state);
    subsets[0] = start_set;
    dfa_add_state(&dfa);   /* state 0 */
    dfa_state_idx[0] = 0;
    dfa.start_state = 0;
    num_subsets = 1;

    /* check if start subset is accepting */
    for (int s = 0; s < nfa->num_states; s++) {
        if (bs_test(start_set, s) && nfa->accept[s]) {
            dfa.accept[0] = true;
            break;
        }
    }

    int queue[256], qh = 0, qt = 0;
    queue[qt++] = 0; /* enqueue subset index 0 */

    while (qh < qt) {
        int sidx = queue[qh++];
        unsigned long long S = subsets[sidx];

        for (int sym = 0; sym < nfa->num_symbols; sym++) {
            /* compute move(S, sym) */
            unsigned long long move_set = 0;
            for (int s = 0; s < nfa->num_states; s++) {
                if (bs_test(S, s)) {
                    move_set |= nfa->transitions[s][sym];
                }
            }
            unsigned long long closure = nfa_epsilon_closure(nfa, move_set);

            /* find or add this subset */
            int found = -1;
            for (int k = 0; k < num_subsets; k++) {
                if (subsets[k] == closure) { found = k; break; }
            }
            if (found < 0 && closure != 0) {
                /* add new subset */
                found = num_subsets++;
                subsets[found] = closure;
                dfa_add_state(&dfa);
                dfa_state_idx[found] = dfa.num_states - 1;
                queue[qt++] = found;
                /* check accept */
                for (int s = 0; s < nfa->num_states; s++) {
                    if (bs_test(closure, s) && nfa->accept[s]) {
                        dfa.accept[dfa_state_idx[found]] = true;
                        break;
                    }
                }
            }

            if (found >= 0) {
                dfa_add_transition(&dfa, dfa_state_idx[sidx], sym, dfa_state_idx[found]);
            }
        }
    }

    return dfa;
}

/* ---------------------------------------------------------------------- */
/* Thompson construction helpers                                          */
/* ---------------------------------------------------------------------- */

NFA nfa_from_symbol(int num_symbols, int symbol) {
    NFA nfa = nfa_create(num_symbols);
    int s0 = nfa_add_state(&nfa);
    int s1 = nfa_add_state(&nfa);
    nfa.start_state = s0;
    nfa.accept[s1] = true;
    nfa.accept_state = s1;
    nfa_add_transition(&nfa, s0, symbol, s1);
    return nfa;
}

/* Union: L(a) ∪ L(b). Creates new NFA from two input NFAs.
 * The input NFAs are copied/merged into a new one (original ones become invalid). */
NFA nfa_thompson_union(NFA *a, NFA *b) {
    NFA nfa = nfa_create(a->num_symbols);
    /* Copy all states from a and b into nfa, with offset */
    int offset_a = 0;
    int offset_b;

    /* Add states from a */
    for (int i = 0; i < a->num_states; i++) {
        nfa_add_state(&nfa);
    }
    offset_b = nfa.num_states;
    /* Add states from b */
    for (int i = 0; i < b->num_states; i++) {
        nfa_add_state(&nfa);
    }

    /* New start and accept */
    int new_start = nfa_add_state(&nfa);
    int new_accept = nfa_add_state(&nfa);
    nfa.start_state = new_start;
    nfa.accept[new_accept] = true;
    nfa.accept_state = new_accept;

    /* Epsilon from new start to old starts */
    nfa_add_epsilon(&nfa, new_start, offset_a + a->start_state);
    nfa_add_epsilon(&nfa, new_start, offset_b + b->start_state);

    /* Epsilon from old accepts to new accept */
    for (int i = 0; i < a->num_states; i++) {
        if (a->accept[i])
            nfa_add_epsilon(&nfa, offset_a + i, new_accept);
    }
    for (int i = 0; i < b->num_states; i++) {
        if (b->accept[i])
            nfa_add_epsilon(&nfa, offset_b + i, new_accept);
    }

    /* Copy transitions from a */
    for (int i = 0; i < a->num_states; i++) {
        for (int s = 0; s < a->num_symbols; s++) {
            for (int j = 0; j < a->num_states; j++) {
                if (bs_test(a->transitions[i][s], j))
                    nfa_add_transition(&nfa, offset_a + i, s, offset_a + j);
            }
        }
        for (int j = 0; j < a->num_states; j++) {
            if (bs_test(a->epsilon[i], j))
                nfa_add_epsilon(&nfa, offset_a + i, offset_a + j);
        }
    }

    /* Copy transitions from b */
    for (int i = 0; i < b->num_states; i++) {
        for (int s = 0; s < b->num_symbols; s++) {
            for (int j = 0; j < b->num_states; j++) {
                if (bs_test(b->transitions[i][s], j))
                    nfa_add_transition(&nfa, offset_b + i, s, offset_b + j);
            }
        }
        for (int j = 0; j < b->num_states; j++) {
            if (bs_test(b->epsilon[i], j))
                nfa_add_epsilon(&nfa, offset_b + i, offset_b + j);
        }
    }

    return nfa;
}

/* Concatenation: a · b */
NFA nfa_thompson_concat(NFA *a, NFA *b) {
    NFA nfa = nfa_create(a->num_symbols);
    int offset_a = 0;

    for (int i = 0; i < a->num_states; i++) {
        nfa_add_state(&nfa);
    }
    int offset_b = nfa.num_states;
    for (int i = 0; i < b->num_states; i++) {
        nfa_add_state(&nfa);
    }

    nfa.start_state = offset_a + a->start_state;

    /* Add epsilon from each accept of a to start of b */
    for (int i = 0; i < a->num_states; i++) {
        if (a->accept[i])
            nfa_add_epsilon(&nfa, offset_a + i, offset_b + b->start_state);
    }

    /* b's accept states become accept */
    for (int i = 0; i < b->num_states; i++) {
        if (b->accept[i]) {
            nfa.accept[offset_b + i] = true;
        }
    }
    nfa.accept_state = offset_b + b->accept_state;

    /* Copy transitions */
    for (int i = 0; i < a->num_states; i++) {
        for (int s = 0; s < a->num_symbols; s++) {
            for (int j = 0; j < a->num_states; j++)
                if (bs_test(a->transitions[i][s], j))
                    nfa_add_transition(&nfa, offset_a + i, s, offset_a + j);
        }
        for (int j = 0; j < a->num_states; j++)
            if (bs_test(a->epsilon[i], j))
                nfa_add_epsilon(&nfa, offset_a + i, offset_a + j);
    }
    for (int i = 0; i < b->num_states; i++) {
        for (int s = 0; s < b->num_symbols; s++) {
            for (int j = 0; j < b->num_states; j++)
                if (bs_test(b->transitions[i][s], j))
                    nfa_add_transition(&nfa, offset_b + i, s, offset_b + j);
        }
        for (int j = 0; j < b->num_states; j++)
            if (bs_test(b->epsilon[i], j))
                nfa_add_epsilon(&nfa, offset_b + i, offset_b + j);
    }

    return nfa;
}

/* Kleene star: a* */
NFA nfa_thompson_star(NFA *a) {
    NFA nfa = nfa_create(a->num_symbols);
    int offset = 0;

    for (int i = 0; i < a->num_states; i++) {
        nfa_add_state(&nfa);
    }

    int new_start = nfa_add_state(&nfa);
    int new_accept = nfa_add_state(&nfa);
    nfa.start_state = new_start;
    nfa.accept[new_accept] = true;
    nfa.accept_state = new_accept;

    /* epsilon from new_start to old start, and to new_accept (empty string) */
    nfa_add_epsilon(&nfa, new_start, offset + a->start_state);
    nfa_add_epsilon(&nfa, new_start, new_accept);

    /* epsilon from each old accept to old start (loop) and to new_accept */
    for (int i = 0; i < a->num_states; i++) {
        if (a->accept[i]) {
            nfa_add_epsilon(&nfa, offset + i, offset + a->start_state);
            nfa_add_epsilon(&nfa, offset + i, new_accept);
        }
    }

    /* Copy transitions */
    for (int i = 0; i < a->num_states; i++) {
        for (int s = 0; s < a->num_symbols; s++) {
            for (int j = 0; j < a->num_states; j++)
                if (bs_test(a->transitions[i][s], j))
                    nfa_add_transition(&nfa, offset + i, s, offset + j);
        }
        for (int j = 0; j < a->num_states; j++)
            if (bs_test(a->epsilon[i], j))
                nfa_add_epsilon(&nfa, offset + i, offset + j);
    }

    return nfa;
}

/* ---------------------------------------------------------------------- */
/* Print                                                                  */
/* ---------------------------------------------------------------------- */

void nfa_print(const NFA *nfa) {
    printf("NFA: %d states, %d symbols, start=%d\n",
           nfa->num_states, nfa->num_symbols, nfa->start_state);
    printf("Accept: ");
    for (int i = 0; i < nfa->num_states; i++)
        if (nfa->accept[i]) printf("%d ", i);
    printf("\nEpsilon:\n");
    for (int i = 0; i < nfa->num_states; i++) {
        if (nfa->epsilon[i]) {
            printf("  %d -> { ", i);
            for (int j = 0; j < nfa->num_states; j++)
                if (bs_test(nfa->epsilon[i], j)) printf("%d ", j);
            printf("}\n");
        }
    }
    printf("Transitions:\n");
    for (int i = 0; i < nfa->num_states; i++) {
        for (int s = 0; s < nfa->num_symbols; s++) {
            if (nfa->transitions[i][s]) {
                printf("  %d --%c--> { ", i, (char)('a' + s));
                for (int j = 0; j < nfa->num_states; j++)
                    if (bs_test(nfa->transitions[i][s], j)) printf("%d ", j);
                printf("}\n");
            }
        }
    }
}
