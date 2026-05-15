#include <stdio.h>
#include <string.h>
#include "../include/regex.h"

int main(void) {
    printf("=== Regex Engine Demo ===\n\n");

    /* Test 1: (a|b)*abb */
    {
        char err[128] = {0};
        printf("Pattern: \"(a|b)*abb\"\n");
        if (!regex_compile("(a|b)*abb", err, sizeof(err))) {
            printf("Compile error: %s\n", err);
        } else {
            const char *tests[] = {
                "abb", "aabb", "babb", "ababb", "bbabb",
                "ab", "ba", "aba", "bb", ""
            };
            for (int i = 0; i < 10; i++) {
                printf("  \"%s\" → %s\n", tests[i],
                       regex_match(tests[i]) ? "MATCH" : "no match");
            }
        }
        printf("\n");
    }

    /* Test 2: a+b+ */
    {
        char err[128] = {0};
        printf("Pattern: \"a+b+\"\n");
        if (!regex_compile("a+b+", err, sizeof(err))) {
            printf("Compile error: %s\n", err);
        } else {
            const char *tests[] = {"ab", "aab", "abb", "aaabbb", "a", "b", "ba", ""};
            for (int i = 0; i < 8; i++) {
                printf("  \"%s\" → %s\n", tests[i],
                       regex_match(tests[i]) ? "MATCH" : "no match");
            }
        }
        printf("\n");
    }

    /* Test 3: a(b|c)*d */
    {
        char err[128] = {0};
        printf("Pattern: \"a(b|c)*d\"\n");
        if (!regex_compile("a(b|c)*d", err, sizeof(err))) {
            printf("Compile error: %s\n", err);
        } else {
            const char *tests[] = {
                "ad", "abd", "acd", "abbbd", "abcbcd", "abcd",
                "a", "d", "abce", "abc"
            };
            for (int i = 0; i < 10; i++) {
                printf("  \"%s\" → %s\n", tests[i],
                       regex_match(tests[i]) ? "MATCH" : "no match");
            }
        }
        printf("\n");
    }

    /* Test 4: Partial matching */
    {
        char err[128] = {0};
        printf("Pattern: \"abb\" (partial match test)\n");
        if (!regex_compile("abb", err, sizeof(err))) {
            printf("Compile error: %s\n", err);
        } else {
            const char *haystack = "xxabbxxabbxx";
            int start, end;
            printf("  Text: \"%s\"\n", haystack);
            if (regex_match_partial(haystack, &start, &end)) {
                printf("  First match: start=%d, end=%d → \"", start, end);
                for (int i = start; i < end; i++) putchar(haystack[i]);
                printf("\"\n");
            }
        }
        printf("\n");
    }

    /* Test 5: (ab)* */
    {
        char err[128] = {0};
        printf("Pattern: \"(ab)*\"\n");
        if (!regex_compile("(ab)*", err, sizeof(err))) {
            printf("Compile error: %s\n", err);
        } else {
            const char *tests[] = {"", "ab", "abab", "ababab", "aba", "a", "b", "abb"};
            for (int i = 0; i < 8; i++) {
                printf("  \"%s\" → %s\n", tests[i],
                       regex_match(tests[i]) ? "MATCH" : "no match");
            }
        }
        printf("\n");
    }

    return 0;
}
