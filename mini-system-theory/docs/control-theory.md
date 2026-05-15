# Control Theory

Fundamentals of classical and modern control theory, with C implementation
references in `mini-system-theory`.

---

## 1. Open-Loop vs Closed-Loop Control

### Open-Loop Control

The controller output is computed solely from the reference signal, with no
measurement of the actual system output.

```
Reference r(t) → [Controller] → u(t) → [Plant G(s)] → y(t)
```

**Properties:**
- Simple, no sensors needed
- No correction for disturbances or model errors
- Requires accurate plant model
- Cannot stabilize unstable plants

```c
// Open-loop: u(t) = K * r(t), no feedback
double u = K * r;
double y = plant_update(u);
// y is NOT fed back to the controller
```

### Closed-Loop (Feedback) Control

The controller uses the error between reference and measured output.

```
r(t) → (+) → e(t) → [Controller C(s)] → u(t) → [Plant G(s)] → y(t)
        ↑ (-)                                              |
        └─────────────────────────────────────────────────┘
```

**Properties:**
- Rejects disturbances
- Robust to model uncertainty
- Can stabilize unstable plants
- May introduce instability (must analyze loop)

| Implementation | File | Key Operations |
|---|---|---|
| Open-loop simulation | `state_space.c` | `ss_simulate` without feedback |
| Closed-loop construction | `control_loop.c` | `cloop_unity_feedback`, `cloop_tf` |
| Disturbance rejection test | `control_loop.c` | `cloop_disturbance_response` |

---

## 2. PID Control

The PID controller is the most widely used feedback controller. Its output is
the sum of three terms:

```
u(t) = Kp * e(t) + Ki * ∫e(τ)dτ + Kd * de/dt
```

### Proportional Term (P)

```
u_P(t) = Kp * e(t)
```

- Produces output proportional to current error
- Larger Kp → faster response, more overshoot, potential instability
- Pure P control leaves steady-state error for non-integrating plants

### Integral Term (I)

```
u_I(t) = Ki * ∫₀ᵗ e(τ) dτ
```

- Accumulates past errors
- Eliminates steady-state error (drives error to zero)
- Too much integral → overshoot, oscillation, integrator windup
- Anti-windup: clamp integral term when actuator saturates

### Derivative Term (D)

```
u_D(t) = Kd * de(t)/dt
```

- Acts on rate of change of error (predictive)
- Adds damping, reduces overshoot
- Amplifies measurement noise (needs low-pass filtering)
- Derivative kick: use derivative-on-measurement to avoid large transients from setpoint changes

| Implementation | File | Key Operations |
|---|---|---|
| PID controller creation | `feedback.c` | `pid_create(Kp, Ki, Kd, dt, max_output)` |
| PID update per time step | `feedback.c` | `pid_update(pid, setpoint, measurement)` |
| Anti-windup (clamping + back-calculation) | `feedback.c` | Built into `pid_update` |
| Derivative-on-measurement | `feedback.c` | `pid_set_deriv_measurement(pid, true)` |
| PID auto-tuning (Ziegler-Nichols) | `feedback.c` | `pid_auto_tune` |

```c
PID *pid = pid_create(1.0, 0.5, 0.1, 0.01, 10.0);
pid_set_deriv_measurement(pid, true);
for (int t = 0; t < 1000; t++) {
    double y = plant.sense();  // measurement
    double u = pid_update(pid, setpoint, y);
    plant.actuate(u);
}
```

### Ziegler-Nichols Tuning Rules

| Type | Kp | Ki (Kp/Ti) | Kd (Kp*Td) |
|---|---|---|---|
| P | 0.5 * Ku | - | - |
| PI | 0.45 * Ku | Kp / (0.83 * Tu) | - |
| PID | 0.6 * Ku | Kp / (0.5 * Tu) | Kp * (0.125 * Tu) |

Where Ku = ultimate gain, Tu = ultimate period (from sustained oscillation at Ku).

---

## 3. Stability (Poles in Left Half-Plane)

A linear time-invariant (LTI) system is **stable** if and only if all poles
(eigenvalues of A, or roots of the denominator of the transfer function) lie
in the open left half of the complex plane: **Re(s) < 0**.

### Why? The impulse response of a pole at s = a + jb is e^(at) * sin(bt + φ).

- If a < 0: response decays to zero → **stable**
- If a = 0: response oscillates indefinitely → **marginally stable**
- If a > 0: response grows unbounded → **unstable**

### Methods to Check Stability

| Method | When to Use | Implementation |
|---|---|---|
| Direct eigenvalue computation | State-space models (small systems) | `stability_eigenvalues(A, n)` |
| Routh-Hurwitz criterion | Transfer functions, no eigenvalues needed | `stability_routh_hurwitz(coeffs, n)` |
| Lyapunov equation | State-space: find P > 0 s.t. A^T P + PA = -Q | `stability_lyapunov_equation(A, n)` |
| Bode plot inspection | Frequency domain, gain/phase margin | `cloop_bode(G)` |
| Root locus | How poles move with gain K | `feedback_root_locus(G, K_range)` |

### Routh-Hurwitz Example

```
For polynomial: s^3 + 2s^2 + 4s + 3
Routh array:
s^3 | 1    4
s^2 | 2    3
s^1 | (2*4 - 1*3)/2 = 2.5    0
s^0 | 3

No sign changes in first column → all roots in LHP → STABLE
```

```c
double coeffs[] = {1, 2, 4, 3}; // s^3 + 2s^2 + 4s + 3
RouthResult r = stability_routh_hurwitz(coeffs, 4);
printf("Stable: %s\n", r.stable ? "yes" : "no");
printf("RHP poles: %d\n", r.n_rhp_poles);
```

---

## 4. Gain Margin and Phase Margin

Stability margins quantify how much "room" exists before a stable closed-loop
system becomes unstable.

### Gain Margin (GM)

```
GM = 1 / |G(jω_pc)|   or in dB: GM_dB = -20 log10|G(jω_pc)|

where ω_pc is the phase crossover frequency: ∠G(jω_pc) = -180°
```

- How much we can increase the gain before instability
- GM > 1 (or GM_dB > 0 dB) → stable closed loop
- Typical requirement: GM > 6 dB

### Phase Margin (PM)

```
PM = 180° + ∠G(jω_gc)

where ω_gc is the gain crossover frequency: |G(jω_gc)| = 1 (0 dB)
```

- How much phase lag we can add before instability
- PM > 0° → stable closed loop
- Typical requirement: PM > 45° (45-60° is good)
- PM ≈ 100 * damping ratio ζ (for second-order dominant systems)

### Implementation

```c
BodeResult b = cloop_bode(loop_tf, 0.01, 100, 1000);
printf("Gain margin:  %.2f dB\n", b.gain_margin_db);
printf("Phase margin: %.2f deg\n", b.phase_margin_deg);
printf("Stable: %s\n", b.gain_margin_db > 0 && b.phase_margin_deg > 0 ? "yes" : "no");
```

| Implementation | File | Key Operations |
|---|---|---|
| Frequency response | `control_loop.c` | `cloop_bode` computes magnitude and phase over frequency range |
| Gain margin | `stability.c` | `stability_gain_margin` at phase crossover |
| Phase margin | `stability.c` | `stability_phase_margin` at gain crossover |
| Bandwidth | `control_loop.c` | `cloop_bandwidth`: frequency where magnitude drops to -3 dB |

---

## 5. Root Locus

Root locus plots the trajectories of closed-loop poles as a single gain
parameter K varies from 0 to ∞.

```
Closed-loop characteristic equation: 1 + K * G(s) * H(s) = 0
```

### Key Rules

1. Branches start at open-loop poles (K = 0) and end at open-loop zeros (K = ∞)
2. Branches lie on the real axis to the left of an odd number of poles + zeros
3. Asymptotes at angles: (2k+1)*180° / (n-m) for k = 0, 1, ..., n-m-1
4. Breakaway/break-in points: solve dK/ds = 0
5. Angle of departure from complex pole, angle of arrival at complex zero

### What Root Locus Tells Us

- **K small:** poles near open-loop poles → system behavior ≈ open loop
- **K medium:** poles move, damping and speed change
- **K large:** poles near open-loop zeros or go to infinity on asymptotes
- **Crossing imaginary axis:** gain at which system goes unstable

```c
TransferFunction *G = tf_create(num, den, 3, 4);
RootLocus *rl = feedback_root_locus(G, 0.001, 1000, 1000);
for (int i = 0; i < rl->n_points; i++) {
    printf("K=%.2f: poles at %.3f%+.3fj\n",
           rl->gains[i],
           creal(rl->poles[i][0]),
           cimag(rl->poles[i][0]));
}
// Find K at instability (pole crosses jω axis)
double K_crit = feedback_root_locus_critical_gain(rl);
```

| Implementation | File | Key Operations |
|---|---|---|
| Root locus computation | `feedback.c` | `feedback_root_locus` traces pole positions |
| Critical gain | `feedback.c` | `feedback_root_locus_critical_gain` |
| Departure angles | `feedback.c` | `feedback_rl_departure_angle` |

---

## 6. State-Space Representation

State-space models represent an nth-order differential equation as a system of
n first-order differential equations.

### Continuous-Time

```
dx/dt = A x(t) + B u(t)    (state equation)
y(t)  = C x(t) + D u(t)    (output equation)
```

- **x(t):** state vector (n×1) — internal memory of the system
- **A:** system matrix (n×n) — determines dynamics
- **B:** input matrix (n×m) — maps inputs to state derivatives
- **C:** output matrix (p×n) — maps states to outputs
- **D:** feedthrough matrix (p×m) — direct input-to-output
- **u(t):** input vector (m×1)
- **y(t):** output vector (p×1)

### Discrete-Time

```
x[k+1] = Ad x[k] + Bd u[k]
y[k]   = Cd x[k] + Dd u[k]
```

| Implementation | File | Key Operations |
|---|---|---|
| Create state-space model | `state_space.c` | `ss_create(A, B, C, D, n_states, n_inputs, n_outputs)` |
| Time simulation (Euler, RK4) | `state_space.c` | `ss_simulate(ss, u_func, t_start, t_end, dt)` |
| Step response | `state_space.c` | `ss_step_response` computes y(t) for u(t) = 1(t) |
| Impulse response | `state_space.c` | `ss_impulse_response` |
| Controllability test | `state_space.c` | `ss_controllability`: rank of controllability matrix |
| Observability test | `state_space.c` | `ss_observability`: rank of observability matrix |
| Controllability matrix | `state_space.c` | C = [B, AB, A^2B, ..., A^(n-1)B] |
| Observability matrix | `state_space.c` | O = [C; CA; CA^2; ...; CA^(n-1)] |
| Convert to transfer function | `state_space.c` | `ss_to_tf`: G(s) = C(sI-A)^(-1)B + D |
| Canonical forms | `state_space.c` | `ss_canonical_form`: controllable, observable, diagonal |
| Discretization | `state_space.c` | `ss_discretize`: ZOH, Tustin (bilinear), forward/backward Euler |
| Luenberger observer | `state_space.c` | `ss_observer_create`, `ss_observer_estimate` |

### Example: Mass-Spring-Damper

```
m * d^2x/dt^2 + c * dx/dt + k * x = F(t)

State vector: x1 = position, x2 = velocity
dx1/dt = x2
dx2/dt = -(k/m)*x1 - (c/m)*x2 + (1/m)*F

A = [0,      1    ]
    [-k/m,  -c/m  ]

B = [0  ]
    [1/m]

C = [1, 0]   (measure position)
D = [0]
```

```c
double A[] = {0, 1, -k/m, -c/m};
double B[] = {0, 1/m};
double C[] = {1, 0};
double D[] = {0};
StateSpace *ss = ss_create(A, B, C, D, 2, 1, 1);

// Step response
ss_step_response(ss, 0.0, 10.0, 0.01, stdout);

// Pole placement: place poles at {-2, -3}
double desired_poles[] = {-2.0, -3.0};
double K[2]; // feedback gain
feedback_pole_placement(ss, desired_poles, K);

// Observer
Observer *obs = ss_observer_create(ss, observer_poles, 3);
```

### Pole Placement via State Feedback

```
u(t) = -K x(t) + r(t)

Closed-loop: dx/dt = (A - B K) x(t) + B r(t)
Eigenvalues of (A - B K) = desired pole locations
```

Requires controllability. Uses Ackermann's formula or Bass-Gura method.

---

## Summary Table

| Concept | Module | Key Functions |
|---|---|---|
| Open-loop vs closed-loop | `control_loop.h` | `cloop_tf`, `cloop_unity_feedback` |
| PID control | `feedback.h` | `pid_create`, `pid_update`, `pid_auto_tune` |
| Stability (eigenvalues) | `stability.h` | `stability_eigenvalues`, `stability_routh_hurwitz` |
| Gain/phase margin | `stability.h`, `control_loop.h` | `cloop_bode`, `stability_gain_margin` |
| Root locus | `feedback.h` | `feedback_root_locus` |
| State-space | `state_space.h` | `ss_create`, `ss_simulate`, `ss_controllability` |
