#include "feedback.h"
#include "state_space.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PI 3.14159265358979323846

void pid_init(PIDController* pid, double Kp, double Ki, double Kd, double dt) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->dt = dt;
    pid->integral = 0;
    pid->prev_error = 0;
    pid->integral_limit = 1e6;
}

double pid_update(PIDController* pid, double setpoint, double measurement) {
    double error = setpoint - measurement;
    pid->integral += error * pid->dt;
    if (pid->integral > pid->integral_limit)
        pid->integral = pid->integral_limit;
    else if (pid->integral < -pid->integral_limit)
        pid->integral = -pid->integral_limit;
    double derivative = (error - pid->prev_error) / pid->dt;
    pid->prev_error = error;
    return pid->Kp * error + pid->Ki * pid->integral + pid->Kd * derivative;
}

void pid_reset(PIDController* pid) {
    pid->integral = 0;
    pid->prev_error = 0;
}

FeedbackLoop* feedback_create(PIDController* pid, StateSpace* plant, double dt) {
    FeedbackLoop* fl = (FeedbackLoop*)malloc(sizeof(FeedbackLoop));
    fl->controller = pid;
    fl->plant = plant;
    fl->x = (double*)calloc(plant->n_states, sizeof(double));
    fl->n_steps = 0;
    fl->dt = dt;
    return fl;
}

void feedback_step(FeedbackLoop* fl, double setpoint, double* output, double* measurement) {
    int n_inputs = fl->plant->n_inputs;
    double* u = (double*)calloc(n_inputs, sizeof(double));

    *measurement = 0;
    if (fl->plant->n_outputs >= 1)
        for (int i = 0; i < fl->plant->n_states; i++)
            *measurement += fl->plant->C[0][i] * fl->x[i];

    u[0] = pid_update(fl->controller, setpoint, *measurement);
    double* y_new = ss_step(fl->plant, fl->x, u, fl->dt);
    *output = y_new[0];

    free(y_new);
    free(u);
    fl->n_steps++;
}

void feedback_simulate(FeedbackLoop* fl, double setpoint, int steps,
                       double* measurements, double* outputs) {
    for (int i = 0; i < steps; i++) {
        double output, measurement;
        feedback_step(fl, setpoint, &output, &measurement);
        measurements[i] = measurement;
        outputs[i] = output;
    }
}

void feedback_free(FeedbackLoop* fl) {
    free(fl->x);
    free(fl);
}

/* ---------- L7: Cascade PID Control ----------
 * Two PID controllers in series: outer loop (position) drives inner loop (velocity)
 * Reference: Astrom & Hagglund (2006) Advanced PID Control
 * (CascadePID struct defined in feedback.h)
 */

void cascade_pid_init(CascadePID* cp, double Kp_o, double Ki_o, double Kd_o,
                      double Kp_i, double Ki_i, double Kd_i, double dt) {
    pid_init(&cp->outer, Kp_o, Ki_o, Kd_o, dt);
    pid_init(&cp->inner, Kp_i, Ki_i, Kd_i, dt);
    cp->outer_output = 0.0;
    cp->inner_output = 0.0;
}

double cascade_pid_update(CascadePID* cp, double setpoint,
                          double outer_meas, double inner_meas) {
    cp->outer_output = pid_update(&cp->outer, setpoint, outer_meas);
    cp->inner_output = pid_update(&cp->inner, cp->outer_output, inner_meas);
    return cp->inner_output;
}

void cascade_pid_reset(CascadePID* cp) {
    pid_reset(&cp->outer);
    pid_reset(&cp->inner);
    cp->outer_output = 0.0;
    cp->inner_output = 0.0;
}

/* ---------- L7: Feedforward Control ----------
 * Combines feedback (PID) with feedforward (model-based) control
 * u_total = u_feedback + u_feedforward
 */
typedef struct {
    PIDController pid;
    double ff_gain;
    double ff_delay;
    double* delay_buffer;
    int delay_idx;
    int delay_len;
} FeedforwardPID;

void ff_pid_init(FeedforwardPID* ff, double Kp, double Ki, double Kd,
                  double ff_gain, double ff_delay, double dt) {
    pid_init(&ff->pid, Kp, Ki, Kd, dt);
    ff->ff_gain = ff_gain;
    ff->ff_delay = ff_delay;

    int steps = (int)(ff_delay / dt) + 1;
    ff->delay_len = steps;
    ff->delay_buffer = (double*)calloc(steps, sizeof(double));
    ff->delay_idx = 0;
}

double ff_pid_update(FeedforwardPID* ff, double setpoint, double measurement,
                     double ff_signal) {
    double fb = pid_update(&ff->pid, setpoint, measurement);

    /* Store feedforward signal in delay buffer */
    ff->delay_buffer[ff->delay_idx] = ff_signal * ff->ff_gain;
    /* Read delayed signal: circular buffer */
    int read_idx = (ff->delay_idx + 1) % ff->delay_len;
    double delayed_ff = ff->delay_buffer[read_idx];
    ff->delay_idx = read_idx;

    return fb + delayed_ff;
}

void ff_pid_reset(FeedforwardPID* ff) {
    pid_reset(&ff->pid);
    for (int i = 0; i < ff->delay_len; i++)
        ff->delay_buffer[i] = 0.0;
}

void ff_pid_free(FeedforwardPID* ff) {
    free(ff->delay_buffer);
}

/* ---------- L7: Anti-windup via Clamping ----------
 * (AntiWindupPID struct defined in feedback.h)
 */
void aw_pid_init(AntiWindupPID* aw, double Kp, double Ki, double Kd,
                 double dt, double out_min, double out_max, double kaw) {
    pid_init(&aw->pid, Kp, Ki, Kd, dt);
    aw->output_min = out_min;
    aw->output_max = out_max;
    aw->back_calculation_gain = kaw;
}

double aw_pid_update(AntiWindupPID* aw, double setpoint, double measurement) {
    /* Standard PID computation */
    double error = setpoint - measurement;
    double P = aw->pid.Kp * error;
    double I = aw->pid.Ki * aw->pid.integral;
    double D = aw->pid.Kd * (error - aw->pid.prev_error) / aw->pid.dt;

    double u_raw = P + I + D;

    /* Saturation */
    double u_sat;
    if (u_raw > aw->output_max) u_sat = aw->output_max;
    else if (u_raw < aw->output_min) u_sat = aw->output_min;
    else u_sat = u_raw;

    /* Anti-windup: back-calculation
     * integral += Kp/Ti * e(t) * dt + Kaw * (u_sat - u_raw) * dt */
    aw->pid.integral += error * aw->pid.dt;
    aw->pid.integral += aw->back_calculation_gain * (u_sat - u_raw) * aw->pid.dt;

    /* Clamp integral */
    if (aw->pid.integral > aw->pid.integral_limit)
        aw->pid.integral = aw->pid.integral_limit;
    else if (aw->pid.integral < -aw->pid.integral_limit)
        aw->pid.integral = -aw->pid.integral_limit;

    aw->pid.prev_error = error;
    return u_sat;
}

void aw_pid_reset(AntiWindupPID* aw) {
    aw->pid.integral = 0;
    aw->pid.prev_error = 0;
}

/* ---------- L7: PID Auto-tuning via Ziegler-Nichols Method ----------
 * Step 1: Ultimate gain Ku (gain at which system oscillates)
 * Step 2: Ultimate period Tu
 * Step 3: Controller parameters from Z-N table
 * Reference: Ziegler & Nichols (1942) "Optimum Settings for Automatic Controllers"
 */
typedef struct {
    double Ku;
    double Tu;
} ZNResult;

void auto_tune_relay_feedback(double** A, double** B, double** C, int n,
                               double h, double relay_amp,
                               double* Ku_out, double* Tu_out) {
    /* Simulate relay feedback to find ultimate gain and period */
    double x[8] = {0};
    double y = 0;
    double relay_out = relay_amp;
    int n_cycles = 0;
    double t = 0;
    double prev_relay = relay_amp;
    double crossings[32];
    int n_crossings = 0;

    for (int step = 0; step < 50000; step++) {
        /* Output */
        y = 0;
        for (int i = 0; i < n; i++) y += C[0][i] * x[i];

        /* Relay: sign of -y (negative feedback) */
        relay_out = (y > 0) ? -relay_amp : relay_amp;

        /* Detect zero crossing */
        if (prev_relay * relay_out < 0 && n_crossings < 32) {
            crossings[n_crossings++] = t;
        }
        prev_relay = relay_out;

        /* State update: xdot = Ax + B*u */
        double dx[8];
        for (int i = 0; i < n; i++) {
            dx[i] = 0;
            for (int j = 0; j < n; j++) dx[i] += A[i][j] * x[j];
            for (int j = 0; j < 1; j++) dx[i] += B[i][j] * relay_out;
            x[i] += dx[i] * h;
        }

        t += h;
        if (t > 50.0 && n_crossings >= 8) break;
    }

    if (n_crossings >= 6) {
        double period_sum = 0;
        int periods = 0;
        for (int i = 2; i < n_crossings - 1; i += 2) {
            period_sum += crossings[i] - crossings[i - 2];
            periods++;
        }
        *Tu_out = (periods > 0) ? period_sum / periods : 1.0;
    } else {
        *Tu_out = 1.0;
    }

    /* Ku = 4*relay_amp / (pi * |y_peak|) */
    *Ku_out = relay_amp * 4.0 / PI;
}

void ziegler_nichols_pid(double Ku, double Tu, double* Kp, double* Ki, double* Kd) {
    /* Classic Z-N tuning rules */
    *Kp = 0.6 * Ku;
    *Ki = 2.0 * (*Kp) / Tu;
    *Kd = (*Kp) * Tu / 8.0;
}

void ziegler_nichols_pi(double Ku, double Tu, double* Kp, double* Ki) {
    *Kp = 0.45 * Ku;
    *Ki = 0.54 * Ku / Tu;
}
