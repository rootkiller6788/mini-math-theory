#ifndef FEEDBACK_H
#define FEEDBACK_H

#include "state_space.h"

typedef struct {
    double Kp, Ki, Kd;
    double integral;
    double prev_error;
    double dt;
    double integral_limit;
} PIDController;

typedef struct {
    PIDController* controller;
    StateSpace* plant;
    double* x;
    int n_steps;
    double dt;
} FeedbackLoop;

void pid_init(PIDController* pid, double Kp, double Ki, double Kd, double dt);
double pid_update(PIDController* pid, double setpoint, double measurement);
void pid_reset(PIDController* pid);

FeedbackLoop* feedback_create(PIDController* pid, StateSpace* plant, double dt);
void feedback_step(FeedbackLoop* fl, double setpoint, double* output, double* measurement);
void feedback_simulate(FeedbackLoop* fl, double setpoint, int steps,
                       double* measurements, double* outputs);
void feedback_free(FeedbackLoop* fl);

/* L7: Ziegler-Nichols PID tuning */
void ziegler_nichols_pid(double Ku, double Tu, double* Kp, double* Ki, double* Kd);
void ziegler_nichols_pi(double Ku, double Tu, double* Kp, double* Ki);

/* L7: Cascade PID control */
typedef struct {
    PIDController outer;
    PIDController inner;
    double outer_output;
    double inner_output;
} CascadePID;

void cascade_pid_init(CascadePID* cp, double Kp_o, double Ki_o, double Kd_o,
                      double Kp_i, double Ki_i, double Kd_i, double dt);
double cascade_pid_update(CascadePID* cp, double setpoint,
                          double outer_meas, double inner_meas);
void cascade_pid_reset(CascadePID* cp);

/* L7: Anti-windup PID */
typedef struct {
    PIDController pid;
    double output_min;
    double output_max;
    double back_calculation_gain;
} AntiWindupPID;

void aw_pid_init(AntiWindupPID* aw, double Kp, double Ki, double Kd,
                 double dt, double out_min, double out_max, double kaw);
double aw_pid_update(AntiWindupPID* aw, double setpoint, double measurement);
void aw_pid_reset(AntiWindupPID* aw);

#endif
