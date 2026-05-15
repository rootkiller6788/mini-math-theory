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
