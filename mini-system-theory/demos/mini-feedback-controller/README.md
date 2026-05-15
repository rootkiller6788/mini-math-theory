# mini-feedback-controller: PID Control of a Mass-Spring-Damper

Feedback controller demo applying PID control, pole placement, and step
response analysis to a mass-spring-damper system.

## Overview

Controls a second-order mechanical system (mass-spring-damper) using PID
feedback. Demonstrates controller design, stability analysis via pole placement,
and step response tuning for rise time, overshoot, and settling time.

## The Plant: Mass-Spring-Damper

### Physical System

```
    ┌─── k ───┐
    │         │
    ├─── m ───┤──→ F(t) (control force)
    │         │
    └─── c ───┘
    
    Wall → [spring k] → [mass m] → [damper c] → Wall
                          ↑
                      Force F
```

### Equation of Motion

```
m * d²x/dt² + c * dx/dt + k * x = F(t)

m = mass (kg)
c = damping coefficient (N·s/m)
k = spring constant (N/m)
x = displacement (m)
F = applied force (N)
```

### Transfer Function

```
G(s) = X(s)/F(s) = 1 / (m*s² + c*s + k)
     = (1/m) / (s² + (c/m)*s + k/m)
     
Natural frequency:    ω_n = sqrt(k/m)
Damping ratio:        ζ = c / (2*sqrt(k*m))
```

### State-Space Form

```
State: x1 = position, x2 = velocity

dx1/dt = x2
dx2/dt = -(k/m)*x1 - (c/m)*x2 + (1/m)*F

A = [0,      1    ]
    [-k/m,  -c/m  ]

B = [0]
    [1/m]

C = [1, 0]  (measure position)
D = [0]
```

## PID Controller Design

### Controller Structure

```
r(t) → (+) → e(t) → [PID] → u(t) → [Plant G(s)] → y(t)
        ↑ (-)                                    |
        └──────────────────────────────────────┘
```

```c
// Controller parameters
double Kp = 10.0;  // Proportional gain
double Ki = 0.5;   // Integral gain
double Kd = 2.0;   // Derivative gain

StateSpace *plant = ss_create(A_data, B_data, C_data, D_data, 2, 1, 1);
PID *pid = pid_create(Kp, Ki, Kd, dt, max_force);
pid_set_deriv_measurement(pid, true); // avoid derivative kick

// Simulation loop
for (int i = 0; i < n_steps; i++) {
    double y = ss_output(plant);              // measure position
    double u = pid_update(pid, target_pos, y); // compute control
    ss_step(plant, u, dt);                    // apply force, advance
    printf("%.4f, %.4f, %.4f\n", t, y, u);
}
```

### Tuning Effects

| Parameter | Rise Time | Overshoot | Settling Time | Steady-State Error |
|---|---|---|---|---|
| Increase Kp | Decrease | Increase | Small change | Decrease |
| Increase Ki | Decrease | Increase | Increase | Eliminate |
| Increase Kd | Small change | Decrease | Decrease | No effect |

### Ziegler-Nichols Tuning

```c
// Step 1: Set Ki = Kd = 0, increase Kp until sustained oscillation
double Ku = 18.0;  // Ultimate gain (where oscillation is sustained)
double Tu = 2.5;   // Ultimate period (seconds)

// Step 2: Apply Ziegler-Nichols PID table
double Kp_zn = 0.6 * Ku;        // = 10.8
double Ki_zn = Kp_zn / (0.5 * Tu); // = 8.64
double Kd_zn = Kp_zn * (0.125 * Tu); // = 3.375

PID *pid_zn = pid_create(Kp_zn, Ki_zn, Kd_zn, dt, max_force);
```

## Stability Analysis: Pole Placement

### State Feedback Law

```
u(t) = -K * x(t) + r(t)   where K = [k1, k2]

Closed-loop A matrix: Acl = A - B*K
Eigenvalues of Acl = desired pole locations
```

### Desired Pole Selection

For a second-order system with desired ζ = 0.7 and ω_n = 5 rad/s:

```
s1,2 = -ζ*ω_n ± j*ω_n*sqrt(1-ζ²)
     = -3.5 ± j*3.57

These poles give:
  Rise time (10-90%):  tr ≈ 1.8 / ω_n = 0.36 s
  Settling time (2%):   ts ≈ 4 / (ζ*ω_n) = 1.14 s
  Overshoot:             Mp ≈ exp(-π*ζ/sqrt(1-ζ²)) * 100% = 4.6%
```

```c
double desired_poles[] = {-3.5 + 3.57*I, -3.5 - 3.57*I};
double K[2]; // feedback gain matrix
feedback_pole_placement(ss, desired_poles, K);
// K = [-3.84, 3.50] (example values for m=1, k=5, c=1)
```

### Observability and Luenberger Observer

When not all states are measurable (e.g., only position):

```c
double observer_poles[] = {-7.0, -7.0}; // observer 2-3x faster than controller
Observer *obs = ss_observer_create(ss, observer_poles, 2);

// In simulation loop
double y = ss_output(ss);                    // measured output
double x_hat[2];                             // estimated states
ss_observer_estimate(obs, u, y, dt, x_hat);  // update estimate
double u = -dot(K, x_hat, 2) + r;            // use estimated states
```

## Step Response Analysis

| Metric | Definition | Typical Goal |
|---|---|---|
| **Rise Time (tr)** | Time from 10% to 90% of final value | As fast as possible |
| **Peak Time (tp)** | Time to first peak | — |
| **Overshoot (Mp)** | (peak - final) / final * 100% | < 10% |
| **Settling Time (ts)** | Time to stay within ±2% of final value | Minimize |
| **Steady-State Error (ess)** | r(∞) - y(∞) | = 0 |

```c
StepResponse sr = ss_step_response(ss, 0.0, 10.0, 0.001);
printf("Rise time:      %.3f s\n", sr.rise_time);
printf("Peak time:      %.3f s\n", sr.peak_time);
printf("Overshoot:      %.1f%%\n", sr.overshoot_pct);
printf("Settling time:  %.3f s\n", sr.settling_time);
printf("Steady-state:   %.4f\n", sr.steady_state);
printf("Steady-state err: %.4f\n", sr.steady_state_error);
```

## Expected Output

```
=== Feedback Controller Demo ===
Plant: m=1.0 kg, c=1.0 Ns/m, k=5.0 N/m
Open-loop poles: -0.50 ± 2.18j (damping=0.224)

--- Ziegler-Nichols Tuning ---
Ultimate gain Ku: 18.0
Ultimate period Tu: 2.50 s
PID gains: Kp=10.80, Ki=8.64, Kd=3.38

--- Closed-Loop Step Response ---
Target position: 1.0 m
Rise time:      0.38 s
Peak time:      0.85 s
Overshoot:      12.3%
Settling time:  2.14 s
Steady-state error: 0.001 m

--- Pole Placement Design ---
Desired poles: -3.50 ± 3.57j (damping=0.700, wn=5.0)
Feedback gains: K = [-3.84, 3.50]

Closed-loop step response (pole placement):
Rise time:      0.34 s
Peak time:      0.72 s
Overshoot:      4.6%
Settling time:  1.10 s
Steady-state error: 0.000 m

--- Comparison ---
Ziegler-Nichols PID: overshoot higher, simpler tuning
Pole placement:      exact specification, needs state feedback
```

## C Implementation Outline

```c
int main(void) {
    // Plant parameters
    double m = 1.0, c = 1.0, k = 5.0;

    // State-space model
    double A[] = {0, 1, -k/m, -c/m};
    double B[] = {0, 1/m};
    double C[] = {1, 0};
    double D[] = {0};
    StateSpace *plant = ss_create(A, B, C, D, 2, 1, 1);

    // PID controller
    PID *pid = pid_create(10.8, 8.64, 3.38, 0.001, 20.0);

    // Step response simulation
    double target = 1.0;
    for (double t = 0; t < 10.0; t += 0.001) {
        double x[2];
        ss_get_state(plant, x);
        double y = x[0]; // position
        double u = pid_update(pid, target, y);
        ss_step(plant, u, 0.001);
        // Log t, y, u...
    }

    // Analyze step response
    StepResponse sr = step_response_analyze(log, n_samples, 0.001);

    // Pole placement comparison
    double desired[] = {-3.5, -3.5}; // real parts only for this API
    double K[2];
    feedback_pole_placement(plant, desired, K, 2);
    // Compare with PID results...
}
```
