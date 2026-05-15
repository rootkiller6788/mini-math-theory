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

#endif
