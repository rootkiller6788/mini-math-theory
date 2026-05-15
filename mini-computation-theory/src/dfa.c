#include "dfa.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ---------------------------------------------------------------------- */
/* Create                                                                 */
/* ---------------------------------------------------------------------- */

DFA dfa_create(int num_symbols) {
    DFA dfa;
    dfa.num_states = 0;
    dfa.num_symbols = num_symbols;
    dfa.start_state = 0;
    memset(dfa.accept, 0, sizeof(dfa.accept));
    memset(dfa.used, 0, sizeof(dfa.used));
    for (int i = 0; i < MAX_DFA_STATES; i++) {
        for (int j = 0; j < MAX_DFA_SYMBOLS; j++) {
            dfa.transitions[i][j] = DFA_DEAD_STATE;
        }
    }
    return dfa;
}

int dfa_add_state(DFA *dfa) {
    for (int i = 0; i < MAX_DFA_STATES; i++) {
        if (!dfa->used[i]) {
            dfa->used[i] = true;
            dfa->accept[i] = false;
            for (int j = 0; j < dfa->num_symbols; j++) {
                dfa->transitions[i][j] = DFA_DEAD_STATE;
            }
            if (i >= dfa->num_states) {
                dfa->num_states = i + 1;
            }
            return i;
        }
    }
    return DFA_DEAD_STATE;
}

void dfa_set_accept(DFA *dfa, int state, bool acc) {
    if (state >= 0 && state < MAX_DFA_STATES) {
        dfa->accept[state] = acc;
    }
}

void dfa_set_start(DFA *dfa, int state) {
    dfa->start_state = state;
}

void dfa_add_transition(DFA *dfa, int s, int c, int next) {
    if (s >= 0 && s < MAX_DFA_STATES && c >= 0 && c < dfa->num_symbols &&
        next >= 0 && next < MAX_DFA_STATES) {
        dfa->transitions[s][c] = next;
    }
}

/* ---------------------------------------------------------------------- */
/* Simulate                                                               */
/* ---------------------------------------------------------------------- */

bool dfa_simulate(const DFA *dfa, const int *input, int len) {
    int state = dfa->start_state;
    for (int i = 0; i < len; i++) {
        int sym = input[i];
        if (sym < 0 || sym >= dfa->num_symbols) return false;
        state = dfa->transitions[state][sym];
        if (state == DFA_DEAD_STATE) return false;
    }
    return dfa->accept[state];
}

/* ---------------------------------------------------------------------- */
/* Minimize (partition refinement)                                        */
/* ---------------------------------------------------------------------- */

void dfa_minimize(DFA *dfa) {
    int n = dfa->num_states;
    int sym = dfa->num_symbols;
    if (n <= 1) return;

    /* group[state] = partition group index */
    int group[MAX_DFA_STATES];
    int next_group[MAX_DFA_STATES];
    int num_groups;

    /* initial partition: accept (0) and non-accept (1), or all one group */
    for (int i = 0; i < n; i++) {
        group[i] = dfa->accept[i] ? 0 : 1;
    }
    num_groups = 2;
    /* check if only accept or only non-accept states exist */
    bool has_accept = false, has_non = false;
    for (int i = 0; i < n; i++) {
        if (dfa->accept[i]) has_accept = true;
        else has_non = true;
    }
    if (!has_accept || !has_non) {
        num_groups = 1;
        for (int i = 0; i < n; i++) group[i] = 0;
    }

    /* refine until stable */
    int iter;
    for (iter = 0; iter < 100; iter++) {
        bool changed = false;
        /* for each state, compute its signature: for each symbol, the group its transition leads to */
        for (int i = 0; i < n; i++) {
            int sig[MAX_DFA_SYMBOLS];
            for (int s = 0; s < sym; s++) {
                int t = dfa->transitions[i][s];
                if (t == DFA_DEAD_STATE) {
                    sig[s] = -1;
                } else if (t >= 0 && t < n) {
                    sig[s] = group[t];
                } else {
                    sig[s] = -1;
                }
            }
            /* compute hash / new group */
            int hash = 0;
            for (int s = 0; s < sym; s++) {
                hash = hash * 31 + (sig[s] + 2);
            }
            next_group[i] = hash;
        }

        /* assign integer group IDs from signatures */
        int new_ids[MAX_DFA_STATES];
        int nid = 0;
        for (int i = 0; i < n; i++) {
            bool found = false;
            for (int j = 0; j < i; j++) {
                if (group[j] == group[i] && next_group[j] == next_group[i]) {
                    new_ids[i] = new_ids[j];
                    found = true;
                    break;
                }
            }
            if (!found) {
                new_ids[i] = nid++;
            }
        }

        /* copy new groups */
        for (int i = 0; i < n; i++) {
            if (new_ids[i] != group[i]) changed = true;
            group[i] = new_ids[i];
        }
        num_groups = nid;

        if (!changed) break;
    }

    /* Build minimized DFA:
     * map: group_id -> new state index */
    int g2s[MAX_DFA_STATES];
    memset(g2s, -1, sizeof(g2s));
    int rep[MAX_DFA_STATES]; /* representative state for each group */
    int new_ns = 0;
    for (int i = 0; i < n; i++) {
        int g = group[i];
        if (g2s[g] < 0) {
            g2s[g] = new_ns;
            rep[g] = i;
            new_ns++;
        }
    }

    DFA new_dfa = dfa_create(sym);
    /* add states */
    for (int i = 0; i < new_ns; i++) {
        dfa_add_state(&new_dfa);
    }
    /* start state */
    new_dfa.start_state = g2s[group[dfa->start_state]];
    /* accept */
    for (int i = 0; i < n; i++) {
        if (dfa->accept[i]) {
            new_dfa.accept[g2s[group[i]]] = true;
        }
    }
    /* transitions */
    for (int g = 0; g < num_groups; g++) {
        int r = rep[g];
        for (int s = 0; s < sym; s++) {
            int t = dfa->transitions[r][s];
            if (t != DFA_DEAD_STATE && t >= 0 && t < n) {
                new_dfa.transitions[g2s[g]][s] = g2s[group[t]];
            }
        }
    }
    new_dfa.num_states = new_ns;

    *dfa = new_dfa;
}

/* ---------------------------------------------------------------------- */
/* Complement                                                             */
/* ---------------------------------------------------------------------- */

DFA dfa_complement(const DFA *dfa) {
    DFA comp = *dfa;
    for (int i = 0; i < comp.num_states; i++) {
        comp.accept[i] = !comp.accept[i];
    }
    return comp;
}

/* ---------------------------------------------------------------------- */
/* Union (product construction)                                           */
/* ---------------------------------------------------------------------- */

DFA dfa_union(const DFA *a, const DFA *b) {
    DFA u = dfa_create(a->num_symbols);
    for (int i = 0; i < a->num_states; i++) {
        for (int j = 0; j < b->num_states; j++) {
            dfa_add_state(&u);
        }
    }
    /* state (i,j) maps to i * b->num_states + j */
    u.start_state = a->start_state * b->num_states + b->start_state;
    for (int i = 0; i < a->num_states; i++) {
        for (int j = 0; j < b->num_states; j++) {
            int sij = i * b->num_states + j;
            u.accept[sij] = a->accept[i] || b->accept[j];
            for (int s = 0; s < a->num_symbols; s++) {
                int ta = a->transitions[i][s];
                int tb = b->transitions[j][s];
                if (ta == DFA_DEAD_STATE || tb == DFA_DEAD_STATE) {
                    u.transitions[sij][s] = DFA_DEAD_STATE;
                } else {
                    u.transitions[sij][s] = ta * b->num_states + tb;
                }
            }
        }
    }
    u.num_states = a->num_states * b->num_states;
    return u;
}

/* ---------------------------------------------------------------------- */
/* Intersection (product construction)                                    */
/* ---------------------------------------------------------------------- */

DFA dfa_intersection(const DFA *a, const DFA *b) {
    DFA inter = dfa_create(a->num_symbols);
    for (int i = 0; i < a->num_states; i++) {
        for (int j = 0; j < b->num_states; j++) {
            dfa_add_state(&inter);
        }
    }
    inter.start_state = a->start_state * b->num_states + b->start_state;
    for (int i = 0; i < a->num_states; i++) {
        for (int j = 0; j < b->num_states; j++) {
            int sij = i * b->num_states + j;
            inter.accept[sij] = a->accept[i] && b->accept[j];
            for (int s = 0; s < a->num_symbols; s++) {
                int ta = a->transitions[i][s];
                int tb = b->transitions[j][s];
                if (ta == DFA_DEAD_STATE || tb == DFA_DEAD_STATE) {
                    inter.transitions[sij][s] = DFA_DEAD_STATE;
                } else {
                    inter.transitions[sij][s] = ta * b->num_states + tb;
                }
            }
        }
    }
    inter.num_states = a->num_states * b->num_states;
    return inter;
}

/* ---------------------------------------------------------------------- */
/* Print                                                                  */
/* ---------------------------------------------------------------------- */

void dfa_print(const DFA *dfa) {
    printf("DFA: %d states, %d symbols, start=%d\n",
           dfa->num_states, dfa->num_symbols, dfa->start_state);
    printf("Accept: ");
    for (int i = 0; i < dfa->num_states; i++)
        if (dfa->accept[i]) printf("%d ", i);
    printf("\nTransitions:\n");
    for (int i = 0; i < dfa->num_states; i++) {
        printf("  state %d: ", i);
        for (int s = 0; s < dfa->num_symbols; s++) {
            int t = dfa->transitions[i][s];
            if (t != DFA_DEAD_STATE)
                printf("%c->%d ", (char)('a' + s), t);
        }
        printf("\n");
    }
}
