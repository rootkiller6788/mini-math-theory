#include "inference.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define N_STEPS 100
#define N_TRIALS 1000
#define PI 3.14159265358979323846

int main(void) {
    srand((unsigned)time(NULL));

    printf("=== 1D Random Walk Simulation ===\n\n");
    printf("Steps: %d, Trials: %d\n\n", N_STEPS, N_TRIALS);

    double* final_positions = (double*)malloc(N_TRIALS * sizeof(double));
    double* final_positions_2d = (double*)malloc(N_TRIALS * sizeof(double));

    for (int trial = 0; trial < N_TRIALS; trial++) {
        int pos = 0;
        for (int step = 0; step < N_STEPS; step++) {
            if (rand() % 2 == 0)
                pos++;
            else
                pos--;
        }
        final_positions[trial] = (double)pos;
    }

    printf("1D Random Walk Results:\n");
    printf("  Mean final position:   %.4f (expected: 0.0)\n", mean(final_positions, N_TRIALS));
    printf("  Variance of position:  %.4f (expected: %d.0)\n",
           variance(final_positions, N_TRIALS), N_STEPS);
    printf("  Std dev of position:   %.4f (expected: %.4f)\n",
           stddev(final_positions, N_TRIALS), sqrt((double)N_STEPS));

    printf("\n=== 2D Random Walk Simulation ===\n\n");
    printf("Steps: %d, Trials: %d\n\n", N_STEPS, N_TRIALS);

    for (int trial = 0; trial < N_TRIALS; trial++) {
        double x = 0.0, y = 0.0;
        for (int step = 0; step < N_STEPS; step++) {
            int dir = rand() % 4;
            switch (dir) {
                case 0: x += 1.0; break;
                case 1: x -= 1.0; break;
                case 2: y += 1.0; break;
                case 3: y -= 1.0; break;
            }
        }
        final_positions_2d[trial] = sqrt(x * x + y * y);
    }

    printf("2D Random Walk Results (distance from origin):\n");
    printf("  Mean distance:         %.4f (expected ~sqrt(pi*n/4)=%.4f)\n",
           mean(final_positions_2d, N_TRIALS), sqrt(PI * N_STEPS / 4.0));

    double rms = 0.0;
    for (int i = 0; i < N_TRIALS; i++) {
        rms += final_positions_2d[i] * final_positions_2d[i];
    }
    rms = sqrt(rms / N_TRIALS);
    printf("  RMS distance:          %.4f (expected: sqrt(n)=%.4f)\n",
           rms, sqrt((double)N_STEPS));

    printf("\n=== Basic Statistics on 1D positions ===\n\n");
    printf("  Median:    %.4f\n", median(final_positions, N_TRIALS));
    printf("  25%% pctile: %.4f\n", percentile(final_positions, N_TRIALS, 25.0));
    printf("  75%% pctile: %.4f\n", percentile(final_positions, N_TRIALS, 75.0));

    free(final_positions);
    free(final_positions_2d);

    return 0;
}
