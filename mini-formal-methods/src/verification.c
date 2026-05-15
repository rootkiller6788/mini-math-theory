#include "verification.h"
#include <stdio.h>
#include <string.h>

void sym_exec_init(SymExecutor* se) {
    se->nvars = 0;
    se->paths_explored = 0;
    se->counterexamples = 0;
}

void sym_exec_add_var(SymExecutor* se, int lo, int hi) {
    if (se->nvars >= MAX_SYM_VARS) return;
    se->vars[se->nvars].value = lo;
    se->vars[se->nvars].is_symbolic = true;
    se->vars[se->nvars].lo = lo;
    se->vars[se->nvars].hi = hi;
    se->vars[se->nvars].current = lo;
    se->nvars++;
}

bool sym_exec_explore(SymExecutor* se,
                       bool (*property)(const int* values),
                       void (*print_ce)(const int* values)) {
    int total = 1;
    for (int i = 0; i < se->nvars; i++)
        total *= (se->vars[i].hi - se->vars[i].lo + 1);

    printf("Symbolic execution: %d vars, %d total paths\n", se->nvars, total);

    bool all_pass = true;
    int values[MAX_SYM_VARS];
    int indices[MAX_SYM_VARS] = {0};

    for (int path = 0; path < total; path++) {
        int t = path;
        for (int i = 0; i < se->nvars; i++) {
            int range = se->vars[i].hi - se->vars[i].lo + 1;
            indices[i] = t % range;
            t /= range;
            values[i] = se->vars[i].lo + indices[i];
        }

        se->paths_explored++;
        if (!property(values)) {
            se->counterexamples++;
            all_pass = false;
            printf("  COUNTEREXAMPLE: ");
            for (int i = 0; i < se->nvars; i++)
                printf("v%d=%d ", i, values[i]);
            printf("\n");
            if (print_ce) print_ce(values);
        }
    }
    return all_pass;
}

void sm_init(StateMachine* sm, int nstates, int initial) {
    sm->nstates = nstates;
    sm->initial = initial;
    sm->is_deterministic = true;
    memset(sm->transitions, -1, sizeof(sm->transitions));
}

void sm_add_transition(StateMachine* sm, int from, int to) {
    sm->transitions[from][to] = 1;
}

bool bmc_check(StateMachine* sm,
               bool (*property)(int state),
               int bound,
               BMCResult* result) {
    static int queue[MAX_STATES * MAX_BMC_STEPS];
    static int parent[MAX_STATES * MAX_BMC_STEPS];
    static bool visited[MAX_BMC_STEPS][MAX_STATES];

    memset(visited, 0, sizeof(visited));
    memset(parent, -1, sizeof(parent));

    int head = 0, tail = 0;
    queue[tail] = sm->initial;
    visited[0][sm->initial] = true;
    parent[0 * MAX_STATES + sm->initial] = -1;
    tail++;

    for (int step = 0; step < bound; step++) {
        int old_tail = tail;
        for (int q = head; q < old_tail; q++) {
            int state = queue[q];
            if (!property(state)) {
                printf("BMC: property violated at state %d (step %d)\n", state, step);
                return false;
            }
            for (int next = 0; next < sm->nstates; next++) {
                if (sm->transitions[state][next] > 0 && !visited[step + 1][next]) {
                    visited[step + 1][next] = true;
                    queue[tail++] = next;
                }
            }
        }
        head = old_tail;
    }
    printf("BMC: property holds up to bound %d\n", bound);
    return true;
}
