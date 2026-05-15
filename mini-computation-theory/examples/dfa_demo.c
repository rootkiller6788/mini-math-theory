#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/dfa.h"

int main(void) {
    printf("=== DFA Demo: Even Number of Ones ===\n\n");

    /* Create DFA with alphabet {0, 1} mapped to symbols 0, 1 */
    DFA even_ones = dfa_create(2);

    int q0 = dfa_add_state(&even_ones);  /* even number of 1's seen */
    int q1 = dfa_add_state(&even_ones);  /* odd number of 1's seen */

    dfa_set_start(&even_ones, q0);
    dfa_set_accept(&even_ones, q0, true);   /* accept when even */

    dfa_add_transition(&even_ones, q0, 0, q0);  /* 0: stay */
    dfa_add_transition(&even_ones, q0, 1, q1);  /* 1: go to odd */
    dfa_add_transition(&even_ones, q1, 0, q1);  /* 0: stay */
    dfa_add_transition(&even_ones, q1, 1, q0);  /* 1: go to even */

    printf("DFA: Even number of 1's\n");
    dfa_print(&even_ones);

    printf("\n--- Simulation ---\n");
    const char *tests[] = {"", "0", "1", "10", "11", "101", "1010", "1111", "0000"};
    for (int i = 0; i < 9; i++) {
        int input[16];
        int len = (int)strlen(tests[i]);
        for (int j = 0; j < len; j++) input[j] = tests[i][j] - '0';
        printf("  \"%s\" → %s\n", tests[i],
               dfa_simulate(&even_ones, input, len) ? "ACCEPT" : "REJECT");
    }

    printf("\n--- Complement ---\n");
    DFA odd_ones = dfa_complement(&even_ones);
    printf("Complement (odd number of 1's):\n");
    dfa_print(&odd_ones);

    /* Second DFA: starts and ends with same symbol (0 or 1) */
    /* This DFA recognizes: 0 Σ* 0  |  1 Σ* 1  |  0  |  1 */
    /* Actually a simpler demo: DFA for "contains at least one 1" */
    DFA has_one = dfa_create(2);
    int a0 = dfa_add_state(&has_one);   /* haven't seen 1 yet */
    int a1 = dfa_add_state(&has_one);   /* have seen 1 */
    dfa_set_start(&has_one, a0);
    dfa_set_accept(&has_one, a1, true);
    dfa_add_transition(&has_one, a0, 0, a0);
    dfa_add_transition(&has_one, a0, 1, a1);
    dfa_add_transition(&has_one, a1, 0, a1);
    dfa_add_transition(&has_one, a1, 1, a1);

    printf("\n--- Second DFA: Contains at least one '1' ---\n");
    dfa_print(&has_one);

    printf("\n--- Union: (even 1's) OR (has one 1) ---\n");
    DFA uni = dfa_union(&even_ones, &has_one);
    dfa_print(&uni);

    printf("\n--- Intersection: (even 1's) AND (has one 1) ---\n");
    DFA inter = dfa_intersection(&even_ones, &has_one);
    dfa_print(&inter);

    printf("\n--- Minimized Even-Ones DFA ---\n");
    dfa_minimize(&even_ones);
    dfa_print(&even_ones);

    return 0;
}
