#include "entropy.h"
#include <stdio.h>
#include <math.h>

int main(void) {
    printf("====== Entropy Demo ======\n\n");

    printf("--- 1. Fair Coin: H = 1 bit ---\n");
    double fair[] = {0.5, 0.5};
    double h_fair = entropy_shannon(fair, 2);
    printf("H(fair coin) = %.4f bits\n\n", h_fair);

    printf("--- 2. Biased Coin: H = -p log p - (1-p) log(1-p) ---\n");
    for (double p = 0.0; p <= 1.01; p += 0.1) {
        printf("H(p=%.1f) = %.4f bits\n", p, entropy_binary(p));
    }
    printf("\n");

    printf("--- 3. 6-sided Die: H = log2(6) ≈ 2.585 ---\n");
    double die[6];
    for (int i = 0; i < 6; i++) die[i] = 1.0 / 6.0;
    printf("H(fair die) = %.4f bits (ideal = %.4f)\n\n",
           entropy_shannon(die, 6), log2(6.0));

    printf("--- 4. Loaded Die ---\n");
    double loaded[] = {0.1, 0.1, 0.1, 0.1, 0.1, 0.5};
    printf("H(loaded) = %.4f bits\n\n", entropy_shannon(loaded, 6));

    printf("--- 5. Text Entropy ---\n");
    const char* texts[] = {
        "aaaaaa",
        "abcdefg",
        "hello world hello world hello world",
        "the quick brown fox jumps over the lazy dog",
    };
    for (int i = 0; i < 4; i++) {
        double h = entropy_of_text(texts[i]);
        printf("\"%s\"\n  H = %.4f bits/char, each char ~%.0f%% of ideal\n",
               texts[i], h, h / 8.0 * 100);
    }
    printf("\n");

    printf("--- 6. Entropy Rate (block size) ---\n");
    const char* text2 = "abababababababab";
    printf("\"abababababababab\":\n");
    for (int bs = 1; bs <= 4; bs *= 2)
        printf("  H (block=%d) = %.4f bits/char\n", bs, entropy_rate(text2, bs));
    printf("  (a,b独立均匀 = 1.0 bit/char)\n");

    return 0;
}
