#include "kl_divergence.h"
#include "entropy.h"
#include <stdio.h>
#include <math.h>

int main(void) {
    printf("====== KL Divergence & Cross-Entropy Demo ======\n\n");

    printf("--- 1. D_KL(P||Q) for Bernoulli ---\n");
    printf("P = fair coin (0.5, 0.5), Q varies:\n");
    double p[2] = {0.5, 0.5};
    for (double q1 = 0.1; q1 <= 0.95; q1 += 0.1) {
        double q[2] = {q1, 1.0 - q1};
        double d = kl_divergence(p, q, 2);
        printf("  D_KL(P || Q=%.1f) = %.6f\n", q1, d);
    }
    printf("\n");

    printf("--- 2. Cross-Entropy = H(P) + D_KL(P||Q) ---\n");
    double q_cross[2] = {0.9, 0.1};
    double ce = cross_entropy(p, q_cross, 2);
    double h_p = entropy_shannon(p, 2);
    double kl = kl_divergence(p, q_cross, 2);
    printf("Cross-Entropy: %.6f\n", ce);
    printf("H(P) + D_KL(P||Q) = %.6f + %.6f = %.6f (match? %s)\n\n",
           h_p, kl, h_p + kl, fabs(ce - h_p - kl) < 1e-10 ? "YES" : "NO");

    printf("--- 3. Perplexity ---\n");
    double ppl = perplexity(p, q_cross, 2);
    printf("Perplexity(P, Q=(0.9,0.1)) = %.4f\n", ppl);
    printf("  (≈ 2^cross_entropy = 2^%.4f)\n\n", ce);

    printf("--- 4. Jensen-Shannon Divergence (symmetric) ---\n");
    double p_a[3] = {0.5, 0.3, 0.2};
    double p_b[3] = {0.2, 0.5, 0.3};
    double js = js_divergence(p_a, p_b, 3);
    double kl_ab = kl_divergence(p_a, p_b, 3);
    double kl_ba = kl_divergence(p_b, p_a, 3);
    printf("D_KL(A||B) = %.6f, D_KL(B||A) = %.6f (ASYMMETRIC)\n", kl_ab, kl_ba);
    printf("JSD(A,B)   = %.6f (SYMMETRIC)\n\n", js);

    printf("--- 5. Smoothed KL (avoid log 0) ---\n");
    double p_s[3] = {1.0, 0.0, 0.0};
    double q_s[3] = {0.0, 1.0, 0.0};
    printf("Raw D_KL diverges (inf)\n");
    printf("Smoothed D_KL(eps=0.01) = %.6f\n",
           kl_divergence_smoothed(p_s, q_s, 3, 0.01));

    return 0;
}
