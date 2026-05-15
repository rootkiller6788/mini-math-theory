# Course Alignment

How `mini-system-theory` modules map to MIT 16.842 (Systems Engineering) and
MIT 6.241J (Dynamic Systems & Control).

---

## MIT 16.842 — Fundamentals of Systems Engineering

MIT 16.842 covers the systems engineering process from conception through
operations, emphasizing the V-model, requirements engineering, architecture
definition, trade studies, and verification & validation.

### Module 1: Systems Engineering Overview and the V-Model

| 16.842 Topic | Our Module | Key Operations |
|---|---|---|
| The V-model lifecycle | `architecture_model.h` | `arch_vmodel_create`, `arch_vmodel_trace` |
| Stakeholder needs → requirements | `architecture_model.h` | `arch_requirements_capture`, `arch_requirements_validate` |
| System decomposition (subsystem breakdown) | `architecture_model.h` | `arch_decompose`, `arch_subsystem_allocate` |
| Verification & validation planning | `architecture_model.h` | `arch_verify`, `arch_validation_plan` |
| Interface definition and management | `architecture_model.h` | `arch_interface_check`, `arch_interface_matrix` |

### Module 2: Requirements Engineering

| 16.842 Topic | Our Module | Key Operations |
|---|---|---|
| Requirements hierarchy (stakeholder → system → subsystem) | `architecture_model.h` | `arch_requirements_matrix` |
| Requirements traceability matrix (RTM) | `architecture_model.h` | `arch_requirements_trace`, `arch_requirements_gap` |
| Writing good requirements (SMART) | `architecture_model.h` | Requirements validation rules |
| Functional vs non-functional requirements | `architecture_model.h` | Categorized requirement structures |
| Requirements change management | `architecture_model.h` | Versioning and impact analysis |

### Module 3: System Architecture and Concept Generation

| 16.842 Topic | Our Module | Key Operations |
|---|---|---|
| Concept generation and selection | `tradespace.h` | `ts_add_alternative`, `ts_rank_alternatives` |
| Functional decomposition (form vs function) | `architecture_model.h` | `arch_decompose`, `arch_functional_flow` |
| Morphological matrix (combinations of subsystem options) | `tradespace.h` | `ts_morphological_matrix` |
| Architecture frameworks (DoDAF, MODAF, TOGAF concepts) | `architecture_model.h` | Interface and relationship modeling |

### Module 4: Trade Space Analysis and Decision Making

| 16.842 Topic | Our Module | Key Operations |
|---|---|---|
| Multi-objective optimization | `tradespace.h` | `ts_create` with multiple objective functions |
| Pareto frontier | `tradespace.h` | `ts_pareto_frontier` |
| Sensitivity analysis | `tradespace.h` | `ts_sensitivity`, `ts_tornado_diagram` |
| Analytical Hierarchy Process (AHP) | `tradespace.h` | `ts_ahp_weight`, `ts_ahp_consistency` |
| Cost-benefit analysis, utility functions | `tradespace.h` | `ts_utility`, `ts_cost_benefit` |
| Kepner-Tregoe decision analysis | `tradespace.h` | Weighted scoring methods |

### Module 5: Integration, Verification, and Validation

| 16.842 Topic | Our Module | Key Operations |
|---|---|---|
| Integration strategies (big-bang vs incremental) | `architecture_model.h` | `arch_integration_plan` |
| Verification methods (test, inspection, analysis, demonstration) | `architecture_model.h` | `arch_verify` |
| Validation (did we build the right system?) | `architecture_model.h` | `arch_validate` |
| Test planning and test coverage | `architecture_model.h` | Requirements coverage matrix |

---

## MIT 6.241J — Dynamic Systems and Control

MIT 6.241J covers linear dynamic systems (inputs, outputs, and state-space
models), stability theory, feedback control design, and optimal control with an
emphasis on rigorous mathematical foundations.

### Part 1: State-Space Models

| 6.241J Topic | Our Module | Key Operations |
|---|---|---|
| State-space representation: x'(t) = Ax(t) + Bu(t) | `state_space.h` | `ss_create`, `ss_simulate` |
| Output equation: y(t) = Cx(t) + Du(t) | `state_space.h` | `ss_set_output` |
| Controllability: rank of [B AB A^2B ...] | `state_space.h` | `ss_controllability` |
| Observability: rank of [C; CA; CA^2; ...] | `state_space.h` | `ss_observability` |
| Transfer function from state-space: G(s) = C(sI-A)^(-1)B + D | `state_space.h` | `ss_to_tf` |
| Canonical forms (controllable, observable, diagonal) | `state_space.h` | `ss_canonical_form` |
| Discretization (continuous → discrete via ZOH, Tustin) | `state_space.h` | `ss_discretize` |

### Part 2: Stability Theory

| 6.241J Topic | Our Module | Key Operations |
|---|---|---|
| Eigenvalue stability criterion | `stability.h` | `stability_eigenvalues` |
| Routh-Hurwitz criterion | `stability.h` | `stability_routh_hurwitz` |
| Lyapunov stability (direct method) | `stability.h` | `stability_lyapunov` |
| Lyapunov equation: A^T P + P A = -Q | `stability.h` | `stability_lyapunov_equation` |
| BIBO (Bounded-Input Bounded-Output) stability | `stability.h` | `stability_bibo` |
| Internal stability vs external stability | `stability.h` | `stability_internal` |
| Pole-zero cancellations and hidden modes | `stability.h` | `stability_detect_hidden_modes` |

### Part 3: Feedback Control

| 6.241J Topic | Our Module | Key Operations |
|---|---|---|
| Closed-loop transfer function: T(s) = G(s)/(1+G(s)H(s)) | `control_loop.h` | `cloop_tf` |
| Unity feedback architecture | `control_loop.h` | `cloop_unity_feedback` |
| PID control (proportional, integral, derivative) | `feedback.h` | `pid_create`, `pid_update` |
| Pole placement via state feedback: u = -Kx | `feedback.h` | `feedback_pole_placement` |
| Root locus analysis | `feedback.h` | `feedback_root_locus` |
| Gain margin and phase margin | `stability.h` | `stability_gain_margin`, `stability_phase_margin` |
| Nyquist stability criterion | `control_loop.h` | `cloop_nyquist` |
| Bode plots and frequency response | `control_loop.h` | `cloop_bode` |
| Loop shaping and bandwidth | `control_loop.h` | `cloop_bandwidth` |
| Observer design (Luenberger observer) | `state_space.h` | `ss_observer_create`, `ss_observer_estimate` |
| Linear Quadratic Regulator (LQR) | `feedback.h` | `feedback_lqr` |

---

## System Dynamics (Meadows, Forrester)

Additional topics drawn from system dynamics literature:

| Concept | Our Module | Key Operations |
|---|---|---|
| Stocks and flows | `system_dynamics.h` | `sd_stock_flow_create`, `sd_simulate` |
| First-order linear systems (exponential growth/decay) | `system_dynamics.h` | `sd_first_order` |
| Second-order systems (oscillations, damping) | `system_dynamics.h` | `sd_second_order` |
| Material/information delays (pipeline delay) | `system_dynamics.h` | `sd_add_delay` |
| Causal loop diagrams | `causal_loop.h` | `cl_create`, `cl_add_link` |
| Reinforcing loops (positive feedback, R) | `causal_loop.h` | `cl_find_loops`, `cl_analyze_polarity` |
| Balancing loops (negative feedback, B) | `causal_loop.h` | `cl_analyze_polarity` |
| Leverage points | `causal_loop.h` | `cl_leverage_points` |

---

## Queueing Theory

| Concept | Our Module | Key Operations |
|---|---|---|
| M/M/c queues (Poisson arrivals, exponential service, c servers) | `queue_model.h` | `queue_mmc_create`, `queue_mmc_metrics` |
| Little's Law: L = λ W | `queue_model.h` | `queue_littles_law` |
| Birth-death processes | `queue_model.h` | `queue_birth_death` |
| Queue simulation (discrete-event) | `queue_model.h` | `queue_simulate` |
| Erlang formulas (blocking probability) | `queue_model.h` | `queue_erlang_b`, `queue_erlang_c` |

---

## Summary Table

| Concept | `mini-system-theory` Module |
|---|---|
| State-space models, controllability, observability | `state_space.h` |
| Stability analysis (eigenvalues, Routh-Hurwitz, Lyapunov) | `stability.h` |
| PID control, pole placement, root locus | `feedback.h` |
| Closed-loop analysis, frequency response, Bode/Nyquist | `control_loop.h` |
| Stock-and-flow modeling, delays, simulation | `system_dynamics.h` |
| Causal loop diagrams, feedback loop analysis | `causal_loop.h` |
| Queueing models (M/M/c, Little's Law) | `queue_model.h` |
| V-model lifecycle, requirements, verification | `architecture_model.h` |
| Trade space, Pareto frontier, multi-objective optimization | `tradespace.h` |
