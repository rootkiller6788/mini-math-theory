# mini-system-theory

MIT 16.842 + 6.241J systems engineering & control theory implemented in C.

A self-contained, zero-dependency C library covering the core topics of systems
engineering (V-model, trade space analysis, requirements) and dynamic systems &
control (state-space models, stability, feedback control), alongside system
dynamics (stocks/flows, causal loops, queueing models).

## Modules

| Module | Header | Description |
|---|---|---|
| **State Space** | `state_space.h` | Linear state-space models, controllability, observability, simulation |
| **Stability** | `stability.h` | Eigenvalue analysis, Routh-Hurwitz criterion, Lyapunov methods |
| **Feedback** | `feedback.h` | PID controller, pole placement, root locus analysis |
| **Control Loop** | `control_loop.h` | Closed-loop transfer functions, gain/phase margin, bandwidth |
| **System Dynamics** | `system_dynamics.h` | Stock-and-flow simulation, first/second-order systems, delays |
| **Causal Loop** | `causal_loop.h` | Causal loop diagrams, reinforcing/balancing loops, polarity analysis |
| **Queue Model** | `queue_model.h` | M/M/c queues, Little's Law, birth-death processes |
| **Architecture Model** | `architecture_model.h` | V-model framework, requirements matrix, interface management |
| **Trade Space** | `tradespace.h` | Multi-objective optimization, Pareto frontier, sensitivity analysis |

## Quick Reference

| Module | Key Functions |
|---|---|
| `state_space.c` | `ss_create`, `ss_simulate`, `ss_controllability`, `ss_observability`, `ss_to_tf`, `ss_step_response` |
| `stability.c` | `stability_eigenvalues`, `stability_routh_hurwitz`, `stability_lyapunov`, `stability_gain_margin`, `stability_phase_margin` |
| `feedback.c` | `pid_create`, `pid_update`, `pid_step_response`, `feedback_pole_placement`, `feedback_root_locus` |
| `control_loop.c` | `cloop_tf`, `cloop_step`, `cloop_bode`, `cloop_bandwidth`, `cloop_nyquist` |
| `system_dynamics.c` | `sd_stock_flow_create`, `sd_simulate`, `sd_first_order`, `sd_second_order`, `sd_add_delay` |
| `causal_loop.c` | `cl_create`, `cl_add_link`, `cl_analyze_polarity`, `cl_find_loops`, `cl_leverage_points` |
| `queue_model.c` | `queue_mmc_create`, `queue_mmc_metrics`, `queue_littles_law`, `queue_birth_death`, `queue_simulate` |
| `architecture_model.c` | `arch_vmodel_create`, `arch_requirements_matrix`, `arch_verify`, `arch_decompose`, `arch_interface_check` |
| `tradespace.c` | `ts_create`, `ts_add_alternative`, `ts_pareto_frontier`, `ts_sensitivity`, `ts_rank_alternatives` |

## Directory Structure

```
mini-system-theory/
├── include/                     Header files (.h)
│   ├── state_space.h
│   ├── stability.h
│   ├── feedback.h
│   ├── control_loop.h
│   ├── system_dynamics.h
│   ├── causal_loop.h
│   ├── queue_model.h
│   ├── architecture_model.h
│   └── tradespace.h
├── src/                         Implementation files (.c)
│   ├── state_space.c
│   ├── stability.c
│   ├── feedback.c
│   ├── control_loop.c
│   ├── system_dynamics.c
│   ├── causal_loop.c
│   ├── queue_model.c
│   ├── architecture_model.c
│   └── tradespace.c
├── examples/                    Standalone example programs
│   ├── feedback_demo.c
│   ├── queue_demo.c
│   ├── stability_demo.c
│   ├── causal_loop_demo.c
│   └── resource_control_demo.c
├── demos/                       Demo READMEs and drivers
│   ├── mini-resource-scheduler/
│   ├── mini-feedback-controller/
│   ├── mini-token-budget-loop/
│   ├── mini-agent-retry-loop/
│   ├── mini-architecture-tradespace/
│   └── mini-complex-system-simulator/
├── tests/                       Unit tests
├── benches/                     Performance benchmarks
├── docs/                        Documentation
│   ├── course-alignment.md
│   ├── systems-thinking.md
│   ├── control-theory.md
│   └── architecture-analysis.md
├── Makefile
└── README.md
```

## Building

### Prerequisites

- C compiler (GCC, Clang, or MSVC)
- GNU Make or compatible

### Build All

```
make all
```

Compiles the static library `libsystem.a` and all examples in `examples/`.

### Run Tests

```
make test
```

Builds and runs all example programs.

### Build a Specific Example

```
make build/feedback_demo
```

### Clean

```
make clean
```

Removes the `build/` directory with all object files, library, and binaries.

## Course Alignment

See [`docs/course-alignment.md`](docs/course-alignment.md) for a detailed
mapping of modules to MIT 16.842 (Systems Engineering) and MIT 6.241J (Dynamic
Systems & Control).
