#include <stdio.h>
#include <string.h>
#include "../include/grammar.h"

static void print_str(const char *str, void *ctx) {
    (void)ctx;
    printf("  \"%s\"\n", str[0] ? str : "ε");
}

int main(void) {
    printf("=== Context-Free Grammar Demo ===\n\n");

    /* Grammar: S → AB, A → aA | ε, B → bB | ε
     * This generates a*b* (any number of a's then any number of b's)
     */
    Grammar g = grammar_create(0); /* S = A, index 0 */
    /* Actually S = index 18 (S), A = 0, B = 1 */
    /* Let's use: S=index 18, A=index 0, B=index 1 */

    g.start_symbol = 18; /* 'S' */
    grammar_add_production(&g, 18, "AB");      /* S → AB */
    grammar_add_production(&g, 0, "aA");       /* A → aA */
    grammar_add_production(&g, 0, "");          /* A → ε */
    grammar_add_production(&g, 1, "bB");       /* B → bB */
    grammar_add_production(&g, 1, "");          /* B → ε */

    printf("Original Grammar:\n");
    grammar_print(&g);

    printf("\n--- Generating strings (up to length 4) ---\n");
    grammar_generate(&g, 4, print_str, NULL);

    printf("\n--- Converting to CNF ---\n");
    grammar_to_cnf(&g);
    grammar_print(&g);

    printf("\n--- CYK Membership Tests ---\n");
    const char *cyk_tests[] = {
        "", "a", "b", "ab", "aab", "abb", "aaabbb", "ba", "aba", "c"
    };
    for (int i = 0; i < 10; i++) {
        int len = (int)strlen(cyk_tests[i]);
        bool in_lang = grammar_cyk(&g, cyk_tests[i], len);
        printf("  \"%s\" → %s\n",
               cyk_tests[i][0] ? cyk_tests[i] : "ε",
               in_lang ? "IN L(G)" : "NOT in L(G)");
    }

    /* Second grammar: balanced parentheses (simplified as {a^n b^n | n>=1}) */
    printf("\n\n=== Second Grammar: S → aSb | ab ===\n");
    Grammar g2 = grammar_create(18); /* S */
    grammar_add_production(&g2, 18, "aSb");   /* S → a S b */
    grammar_add_production(&g2, 18, "ab");    /* S → a b */

    printf("Original:\n");
    grammar_print(&g2);

    grammar_to_cnf(&g2);
    printf("\nCNF:\n");
    grammar_print(&g2);

    printf("\nCYK Tests:\n");
    const char *cyk2_tests[] = {"ab", "aabb", "aaabbb", "aaaabbbb", "aab", "abb", ""};
    for (int i = 0; i < 7; i++) {
        int len = (int)strlen(cyk2_tests[i]);
        bool in_lang = grammar_cyk(&g2, cyk2_tests[i], len);
        printf("  \"%s\" → %s\n",
               cyk2_tests[i][0] ? cyk2_tests[i] : "ε",
               in_lang ? "IN L(G)" : "NOT in L(G)");
    }

    return 0;
}
