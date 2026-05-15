#include "state_space.h"
#include "feedback.h"
#include "stability.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(void) {
    printf("=== Feedback Demo: Mass-Spring-Damper with PID ===\n\n");

    double m = 1.0, c = 0.5, k = 2.0;
    StateSpace* plant = ss_create_mass_spring_damper(m, c, k);
    printf("Plant: m=%.1f, c=%.1f, k=%.1f\n", m, c, k);
    ss_print(plant);

    double* evals = ss_eigenvalues(plant);
    printf("\nOpen-loop eigenvalues:\n");
    for (int i = 0; i < plant->n_states; i++)
        printf("  %.6f %+.6fi\n", evals[2 * i], evals[2 * i + 1]);
    printf("Open-loop stable: %s\n\n", ss_is_stable(plant) ? "Yes" : "No");
    free(evals);

    double dt = 0.01;
    PIDController pid;
    pid_init(&pid, 10.0, 2.0, 1.0, dt);
    printf("PID Controller: Kp=10, Ki=2, Kd=1, dt=%.3f\n\n", dt);

    FeedbackLoop* fl = feedback_create(&pid, plant, dt);
    double setpoint = 1.0;
    int steps = (int)(5.0 / dt);
    double* measurements = (double*)malloc(steps * sizeof(double));
    double* outputs = (double*)malloc(steps * sizeof(double));

    feedback_simulate(fl, setpoint, steps, measurements, outputs);

    printf("Step Response (setpoint = 1.0):\n");
    printf("  t=1s: position = %.6f\n", measurements[(int)(1.0 / dt)]);
    printf("  t=3s: position = %.6f\n", measurements[(int)(3.0 / dt)]);
    printf("  t=5s: position = %.6f\n", measurements[(int)(5.0 / dt)]);

    double final_pos = measurements[steps - 1];
    printf("\nSteady-state: %.6f (error = %.6f)\n\n", final_pos, setpoint - final_pos);

    double** Ac = (double**)malloc(plant->n_states * sizeof(double*));
    for (int i = 0; i < plant->n_states; i++) {
        Ac[i] = (double*)calloc(plant->n_states, sizeof(double));
        for (int j = 0; j < plant->n_states; j++) {
            Ac[i][j] = plant->A[i][j] - pid.Kp * plant->B[i][0] * plant->C[0][j];
        }
    }

    double cl_poles[4];
    double trace = Ac[0][0] + Ac[1][1];
    double det = Ac[0][0] * Ac[1][1] - Ac[0][1] * Ac[1][0];
    double disc = trace * trace - 4 * det;
    if (disc >= 0) {
        cl_poles[0] = (trace + sqrt(disc)) / 2.0;
        cl_poles[1] = 0;
        cl_poles[2] = (trace - sqrt(disc)) / 2.0;
        cl_poles[3] = 0;
    } else {
        cl_poles[0] = trace / 2.0;
        cl_poles[1] = sqrt(-disc) / 2.0;
        cl_poles[2] = trace / 2.0;
        cl_poles[3] = -sqrt(-disc) / 2.0;
    }

    printf("Closed-loop (P-only A - B*Kp*C) eigenvalues:\n");
    for (int i = 0; i < 2; i++)
        printf("  %.6f %+.6fi\n", cl_poles[2 * i], cl_poles[2 * i + 1]);
    printf("Closed-loop stable: %s\n",
           is_stable_poles(cl_poles, 2) ? "Yes" : "No");

    for (int i = 0; i < plant->n_states; i++) free(Ac[i]);
    free(Ac);
    free(measurements);
    free(outputs);
    feedback_free(fl);
    ss_free(plant);

    printf("\nDone.\n");
    return 0;
}
