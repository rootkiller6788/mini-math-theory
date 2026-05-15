#include "control_loop.h"
#include "system_dynamics.h"
#include "feedback.h"
#include "state_space.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(void) {
    printf("=== Resource Control Demo ===\n\n");

    printf("--- Token Bucket Rate Limiter ---\n");
    RateLimiter rl;
    ratelimit_init(&rl, 2.0, 0.1);
    double inputs[] = {0.5, 1.0, 2.0, 5.0, 10.0, 3.0, 1.0, 0.5};
    int n_in = 8;
    printf("  Max rate: 2.0/sec, dt=0.1 (max change=0.2/step)\n");
    printf("  Input -> Output:\n");
    for (int i = 0; i < n_in; i++) {
        double out = ratelimit_update(&rl, inputs[i]);
        printf("    %6.2f -> %6.2f\n", inputs[i], out);
    }
    printf("  Large jumps are limited to the maximum rate.\n\n");

    printf("--- Bang-Bang Temperature Controller ---\n");
    BangBang bb;
    bangbang_init(&bb, 25.0, 23.0);
    printf("  On > 25.0 C, Off < 23.0 C (hysteresis = 2.0 C)\n");
    double temps[] = {20, 22, 24, 26, 27, 26, 25, 24, 23, 22, 21};
    printf("  Temp  -> Heater State:\n");
    for (int i = 0; i < 11; i++) {
        bool state = bangbang_update(&bb, temps[i]);
        printf("    %.1f C -> %s\n", temps[i], state ? "ON " : "OFF");
    }
    printf("  Hysteresis prevents rapid cycling around setpoint.\n\n");

    printf("--- Exponential Smoothing for Noise Filtering ---\n");
    double alpha = 0.2;
    double smooth = 10.0;
    double noisy[] = {10.2, 9.5, 12.1, 8.7, 11.8, 9.1, 10.9, 10.1, 9.8, 10.5};
    int n_noisy = 10;
    printf("  alpha=%.1f, initial=%.1f\n", alpha, smooth);
    printf("  Raw  -> Smoothed:\n");
    for (int i = 0; i < n_noisy; i++) {
        double s = exp_smooth_update(&smooth, alpha, noisy[i]);
        printf("    %5.1f -> %7.3f\n", noisy[i], s);
    }
    printf("  Smoothing removes high-frequency noise.\n\n");

    printf("--- PID vs On-Off Setpoint Tracking ---\n");
    double dt = 0.05;
    int steps = 200;
    double setpoint = 1.0;

    double plant_tc = 0.5;
    StateSpace* ss_pid = ss_create_pid_plant(1.0, plant_tc);
    StateSpace* ss_onoff = ss_create_pid_plant(1.0, plant_tc);

    PIDController pid;
    pid_init(&pid, 3.0, 1.0, 0.5, dt);

    FeedbackLoop* fl_pid = feedback_create(&pid, ss_pid, dt);

    BangBang bb2;
    bangbang_init(&bb2, setpoint + 0.05, setpoint - 0.05);

    double pid_meas[200], pid_out[200];
    feedback_simulate(fl_pid, setpoint, steps, pid_meas, pid_out);

    double onoff_state = 0;
    printf("  t     PID_pos    OnOff_pos   Setpoint\n");
    double bb_state = false;
    for (int i = 0; i < steps; i += 20) {
        double u[1];
        if (bb_state && onoff_state > setpoint + 0.05)
            bb_state = false;
        else if (!bb_state && onoff_state < setpoint - 0.05)
            bb_state = true;
        u[0] = bb_state ? 1.5 : 0.0;
        double* y = ss_step(ss_onoff, &onoff_state, u, dt);
        free(y);

        double t = i * dt;
        printf("  %5.1f  %9.4f  %9.4f  %9.4f\n",
               t, pid_meas[i], onoff_state, setpoint);
    }

    printf("  PID: smooth tracking with small steady-state error.\n");
    printf("  On-Off: oscillations around setpoint (cycling).\n\n");

    printf("--- System Dynamics: Stock-and-Flow ---\n");
    Stock bank;
    stock_init(&bank, 1000.0, 2);
    bank.inflows[0] = 50.0;
    bank.outflows[0] = 30.0;
    bank.inflows[1] = 0.0;
    bank.outflows[1] = 0.0;
    printf("  Bank account: initial=$1000, income=$50/mo, expenses=$30/mo\n");
    printf("  Month   Balance\n");
    for (int m = 0; m <= 12; m += 3) {
        for (int i = 0; i < 3; i++)
            stock_update(&bank, 1.0);
        printf("  %5d   $%8.2f\n", m + 3, bank.level);
    }
    printf("  Net flow of +$20/month accumulates linearly.\n");

    printf("\n--- First-Order Approach to Goal ---\n");
    FirstOrder fo;
    fo_init(&fo, 0.0, 100.0, 2.0);
    printf("  Initial=0, Goal=100, Time constant=2.0\n");
    printf("  Time   Value\n");
    for (int i = 0; i <= 10; i++) {
        printf("  %4.1f   %7.3f\n", i * 0.5, fo.x);
        fo_update(&fo, 0.5);
    }
    printf("  At t=tau (2.0): reaches 63.2%% of goal.\n");

    free(bank.inflows);
    free(bank.outflows);
    feedback_free(fl_pid);
    ss_free(ss_pid);
    ss_free(ss_onoff);

    printf("\nDone.\n");
    return 0;
}
