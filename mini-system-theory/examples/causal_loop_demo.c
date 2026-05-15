#include "causal_loop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    printf("=== Causal Loop Diagram Demo ===\n\n");

    CausalLoopDiagram* cld = cld_create(4);
    strcpy(cld->node_names[0], "Population");
    strcpy(cld->node_names[1], "Births");
    strcpy(cld->node_names[2], "Deaths");
    strcpy(cld->node_names[3], "Food");

    cld_add_link(cld, 0, 1, +1.0);
    cld_add_link(cld, 1, 0, +1.0);

    cld_add_link(cld, 0, 2, +1.0);
    cld_add_link(cld, 2, 0, -1.0);

    cld_add_link(cld, 0, 3, -1.0);
    cld_add_link(cld, 3, 0, -1.0);

    cld_add_link(cld, 3, 1, +1.0);

    printf("Built Causal Loop Diagram:\n");
    printf("  Population -> Births [+]\n");
    printf("  Births -> Population [+]\n");
    printf("  Population -> Deaths [+]\n");
    printf("  Deaths -> Population [-]\n");
    printf("  Population -> Food [-]\n");
    printf("  Food -> Population [-]\n");
    printf("  Food -> Births [+]\n\n");

    cld_print_adjacency(cld);
    printf("\n");

    cld_print_loops(cld);
    printf("\n");

    printf("Feedback Loop Analysis:\n");
    int loop_pop_births[] = {0, 1};
    int result1 = cld_is_reinforcing(cld, loop_pop_births, 2);
    printf("  Population<->Births: %s (polarity product %+d)\n",
           result1 > 0 ? "REINFORCING (+)" : "BALANCING (-)", result1);

    int loop_pop_deaths[] = {0, 2};
    int result2 = cld_is_reinforcing(cld, loop_pop_deaths, 2);
    printf("  Population<->Deaths: %s (polarity product %+d)\n",
           result2 > 0 ? "REINFORCING (+)" : "BALANCING (-)", result2);

    int loop_pop_food[] = {0, 3};
    int result3 = cld_is_reinforcing(cld, loop_pop_food, 2);
    printf("  Population<->Food: %s (polarity product %+d)\n",
           result3 > 0 ? "REINFORCING (+)" : "BALANCING (-)", result3);

    int loop_birth_food_pop[] = {3, 1, 0};
    int result4 = cld_is_reinforcing(cld, loop_birth_food_pop, 3);
    printf("  Food->Births->Population->Food: %s (polarity product %+d)\n",
           result4 > 0 ? "REINFORCING (+)" : "BALANCING (-)", result4);

    printf("\nInterpretation:\n");
    printf("  Reinforcing loops (+) drive exponential growth or decline.\n");
    printf("  Balancing loops (-) drive stabilization toward equilibrium.\n");

    cld_free(cld);
    printf("\nDone.\n");
    return 0;
}
