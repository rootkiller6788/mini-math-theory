#include "channel.h"
#include "entropy.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(void) {
    printf("====== Channel Simulator Demo ======\n\n");
    srand((unsigned)time(NULL));

    printf("--- 1. Binary Symmetric Channel (BSC) ---\n");
    for (double eps = 0.0; eps <= 0.55; eps += 0.1) {
        BinarySymmetricChannel bsc;
        bsc_init(&bsc, eps);
        double cap = bsc_capacity(&bsc);
        printf("p_err=%.1f: Capacity = %.4f bits/channel use\n", eps, cap);
    }
    printf("\n");

    printf("--- 2. BSC Transmission Simulation ---\n");
    BinarySymmetricChannel bsc;
    bsc_init(&bsc, 0.1);
    int n_bits = 1000;
    bool* sent = malloc(n_bits * sizeof(bool));
    bool* received = malloc(n_bits * sizeof(bool));

    for (int i = 0; i < n_bits; i++) sent[i] = rand() % 2;
    bsc_transmit(&bsc, sent, n_bits, received);

    int errors = 0;
    for (int i = 0; i < n_bits; i++)
        if (sent[i] != received[i]) errors++;

    printf("Sent %d bits with error prob 0.1\n", n_bits);
    printf("Received with %d errors (rate=%.3f, expected 0.1)\n\n",
           errors, (double)errors / n_bits);

    free(sent); free(received);

    printf("--- 3. Capacity vs Error Rate ---\n");
    printf(" BSC(p) capacity C = 1 - H(p):\n");
    for (double p = 0.0; p <= 1.0; p += 0.05) {
        double cap = 1.0 - entropy_binary(p);
        printf("  p=%.2f: C=%.4f\n", p, cap);
    }
    printf("\n");

    printf("--- 4. Mutual Information of BSC with uniform input ---\n");
    for (double eps = 0.1; eps <= 0.5; eps += 0.1)
        printf("Input p_x=0.5, error=%.1f: I(X;Y) = %.4f bits\n",
               eps, bsc_mutual_information(eps, 0.5));
    printf("\n");

    printf("--- 5. General Channel Capacity (Blahut-Arimoto) ---\n");
    GeneralChannel gc;
    channel_init(&gc, 2, 2);
    channel_set_transition(&gc, 0, 0, 0.9);
    channel_set_transition(&gc, 0, 1, 0.1);
    channel_set_transition(&gc, 1, 0, 0.1);
    channel_set_transition(&gc, 1, 1, 0.9);

    double cap_ba = channel_capacity_blahut(&gc, 1e-4, 50);
    double cap_uni = channel_capacity_uniform_input(&gc);
    printf("BSC(0.1): capacity via Blahut-Arimoto = %.4f\n", cap_ba);
    printf("         : capacity uniform input    = %.4f\n", cap_uni);
    printf("         : theoretical C = 1-H(0.1)  = %.4f\n", 1.0 - entropy_binary(0.1));
    channel_free(&gc);

    return 0;
}
