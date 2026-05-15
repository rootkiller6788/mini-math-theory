#include <stdio.h>
#include <string.h>
#include "../include/turing_machine.h"

/* Convert a string to symbol array using mapping:
 *   a→1, b→2, #→3, 0→1, 1→2 */
static int map_char_to_sym(char c, char mode) {
    if (mode == 'w') {   /* w#w^R mode */
        if (c == 'a') return 1;
        if (c == 'b') return 2;
        if (c == '#') return 3;
    }
    if (mode == 'i') {   /* increment mode */
        if (c == '0') return 1;
        if (c == '1') return 2;
    }
    if (mode == 'n') {   /* a^n b^n mode */
        if (c == 'a') return 1;
        if (c == 'b') return 2;
    }
    return 0;
}

static void test_tm(const char *label, const TuringMachine *tm,
                    const char *input, char mode) {
    (void)label;
    int symbols[256];
    int len = (int)strlen(input);
    for (int i = 0; i < len; i++) {
        symbols[i] = map_char_to_sym(input[i], mode);
    }
    int result = tm_simulate(tm, symbols, len, 5000);
    const char *status = (result == 1) ? "ACCEPT" :
                         (result == 0) ? "REJECT" : "TIMEOUT";
    printf("  %-20s → %s\n", input, status);
}

int main(void) {
    printf("=== Turing Machine Demo ===\n\n");

    /* --- a^n b^n recognizer --- */
    printf("--- TM: a^n b^n Recognizer ---\n");
    printf("Expect ACCEPT: a^n b^n (n>=1), REJECT: others\n");
    TuringMachine anbn = tm_create_anbn();
    const char *tests_anbn[] = {
        "ab", "aabb", "aaabbb", "aaaabbbb",
        "a", "b", "ba", "aba", "abb", "aab", ""
    };
    for (int i = 0; i < 11; i++) {
        test_tm("anbn", &anbn, tests_anbn[i], 'n');
    }

    /* --- w#w^R recognizer --- */
    printf("\n--- TM: w#w^R Recognizer ---\n");
    printf("Expect ACCEPT: w#w^R (palindrome), REJECT: others\n");
    TuringMachine wwr = tm_create_wwr();
    const char *tests_wwr[] = {
        "#", "a#a", "b#b", "ab#ba", "ba#ab", "aba#aba",
        "a#b", "b#a", "ab#ab", "a##a", "a#", "#a"
    };
    for (int i = 0; i < 12; i++) {
        test_tm("wwr", &wwr, tests_wwr[i], 'w');
    }

    /* --- Binary increment --- */
    printf("\n--- TM: Binary Increment ---\n");
    printf("Computes input + 1 in binary\n");
    TuringMachine inc = tm_create_increment();
    /* For increment, we simulate and print tape content */
    const char *tests_inc[] = {"0", "1", "10", "11", "100", "111", "1011"};
    for (int i = 0; i < 7; i++) {
        int symbols[256];
        int len = (int)strlen(tests_inc[i]);
        for (int j = 0; j < len; j++) {
            symbols[j] = map_char_to_sym(tests_inc[i][j], 'i');
        }
        int result = tm_simulate(&inc, symbols, len, 5000);
        printf("  \"%s\" + 1 → %s\n", tests_inc[i],
               result == 1 ? "ACCEPT (computation complete)" :
               result == 0 ? "REJECT" : "TIMEOUT");
    }

    return 0;
}
