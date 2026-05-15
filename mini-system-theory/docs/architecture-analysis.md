# Systems Architecture Analysis

Core concepts in systems architecture analysis, as taught in MIT 16.842, mapped
to C implementations in `mini-system-theory`.

---

## 1. The V-Model

The V-model is the foundational lifecycle model in systems engineering. It
depicts the relationship between the development phases on the left side of
the V (decomposition) and the corresponding verification & validation phases
on the right side (integration).

```
       Requirements          ←→       Validation
              \                          /
          Architecture     ←→     Verification
                \                    /
           Design         ←→    Integration
                  \              /
             Implementation → Unit Testing
```

### Left Side (Decomposition — Top-Down)

| Phase | Description | Implementation |
|---|---|---|
| **Stakeholder Needs** | What does the customer want? Problem statement | `arch_vmodel_add_phase(model, "needs")` |
| **System Requirements** | What must the system do? Functional + non-functional requirements | `arch_requirements_capture` — structured requirement objects |
| **System Architecture** | How is it structured? Subsystems, interfaces, allocations | `arch_decompose` — hierarchical decomposition tree |
| **Subsystem Design** | Detailed design of each component | `arch_subsystem_allocate` — requirements to subsystems |
| **Implementation** | Build, code, fabricate | Individual module construction |

### Right Side (Integration — Bottom-Up)

| Phase | Description | Implementation |
|---|---|---|
| **Unit Testing** | Test individual modules | `arch_verify_unit` |
| **Integration Testing** | Test interfaces between modules | `arch_verify_integration` |
| **System Verification** | Does the system meet its requirements? | `arch_verify` — requirements verification matrix |
| **System Validation** | Does the system solve the stakeholder's problem? | `arch_validate` — validation scenarios |
| **Operations** | Field deployment, maintenance, eventual disposal | Lifecycle tracking |

```c
VModel *vm = arch_vmodel_create("satellite_attitude_control");

// Left side (decomposition)
arch_vmodel_add_phase(vm, VM_STAKEHOLDER_NEEDS);
arch_vmodel_add_phase(vm, VM_SYSTEM_REQUIREMENTS);
arch_vmodel_add_phase(vm, VM_SYSTEM_ARCHITECTURE);
arch_vmodel_add_phase(vm, VM_SUBSYSTEM_DESIGN);

// Right side (integration)
arch_vmodel_add_phase(vm, VM_UNIT_TESTING);
arch_vmodel_add_phase(vm, VM_INTEGRATION_TESTING);
arch_vmodel_add_phase(vm, VM_SYSTEM_VERIFICATION);
arch_vmodel_add_phase(vm, VM_SYSTEM_VALIDATION);

// Traceability: requirements → verification
arch_vmodel_trace(vm, REQ_ATTITUDE_ACCURACY, TEST_POINTING_PRECISION);
```

| Implementation | File | Key Operations |
|---|---|---|
| V-model lifecycle creation | `architecture_model.c` | `arch_vmodel_create` |
| Phase management | `architecture_model.c` | `arch_vmodel_add_phase`, `arch_vmodel_link` |
| Traceability mapping | `architecture_model.c` | `arch_vmodel_trace` |
| Completeness check | `architecture_model.c` | `arch_vmodel_check_coverage` |

---

## 2. Trade Space Analysis (Pareto Frontier)

Trade space analysis evaluates architectural alternatives across multiple
conflicting objectives to identify the set of non-dominated (Pareto-optimal)
solutions.

### Key Concepts

- **Objective functions:** Metrics to optimize (minimize cost, maximize performance, maximize reliability)
- **Pareto dominance:** Alternative A dominates B if A is at least as good as B in all objectives AND strictly better in at least one
- **Pareto frontier:** Set of all non-dominated alternatives
- **Utopia point:** Ideal (typically unattainable) point with best value of each objective
- **Knee point:** Point on Pareto frontier with maximum marginal benefit per unit cost

```
         Performance
             ↑
             |     * C (dominated by B — same perf, higher cost)
        100% |  * B
             |   /|  * D
             |  /    ~~~~~~~~ Pareto frontier ~~~~~~~~
             | / * A
             |/
             +-----------------→  Cost
                     * E (dominated by A and B)
```

### Implementation

```c
TradeSpace *ts = ts_create(3); // 3 objectives
ts_set_objective(ts, 0, "cost",           TS_MINIMIZE);
ts_set_objective(ts, 1, "performance",    TS_MAXIMIZE);
ts_set_objective(ts, 2, "reliability",    TS_MAXIMIZE);

// Add alternatives
ts_add_alternative(ts, "design_A", (double[]){100, 0.80, 0.95});
ts_add_alternative(ts, "design_B", (double[]){150, 0.95, 0.98});
ts_add_alternative(ts, "design_C", (double[]){175, 0.90, 0.97});
ts_add_alternative(ts, "design_D", (double[]){200, 0.99, 0.99});
ts_add_alternative(ts, "design_E", (double[]){120, 0.70, 0.92});

ParetoFront *front = ts_pareto_frontier(ts);
for (int i = 0; i < front->n_solutions; i++) {
    printf("Pareto-optimal: %s\n", front->names[i]);
}

// Sensitivity analysis
SensitivityResult *sr = ts_sensitivity(ts, "cost");
for (int i = 0; i < sr->n_alternatives; i++) {
    printf("%s: rank change = %.2f\n", sr->names[i], sr->rank_change[i]);
}
```

| Implementation | File | Key Operations |
|---|---|---|
| Trade space creation | `tradespace.c` | `ts_create(n_objectives)` |
| Add alternatives | `tradespace.c` | `ts_add_alternative` |
| Pareto frontier computation | `tradespace.c` | `ts_pareto_frontier` (O(n^2 * m) naive, O(n log n) for 2D) |
| Compute utopia/nadir points | `tradespace.c` | `ts_utopia_point` |
| Sensitivity analysis | `tradespace.c` | `ts_sensitivity` |
| Tornado diagram data | `tradespace.c` | `ts_tornado_diagram` |
| Weighted sum ranking | `tradespace.c` | `ts_rank_alternatives` |
| AHP (Analytic Hierarchy Process) | `tradespace.c` | `ts_ahp_weight`, `ts_ahp_consistency` |

---

## 3. Requirements Verification Matrix

The Requirements Verification Matrix (RVM) maps each system requirement to
one or more verification methods, ensuring complete coverage.

### Verification Methods

| Method | Description | When to Use |
|---|---|---|
| **Test (T)** | Execute under controlled conditions and measure results | Performance, functional requirements |
| **Inspection (I)** | Visual examination, review of documentation | Physical characteristics, workmanship |
| **Analysis (A)** | Mathematical modeling, simulation | Reliability, thermal, structural |
| **Demonstration (D)** | Show operation under specified conditions | Operational scenarios, usability |

### Requirements Traceability Matrix (RTM)

The RTM connects requirements upward to stakeholder needs and downward to
verification events, ensuring bidirectional traceability.

```
Stakeholder Need SN-001 "Navigate accurately"
    └── Requirement REQ-001 "Position accuracy ≤ 10m CEP"
        ├── Subsystem-SS01: GPS receiver
        │   └── Verification V-001: Test with GPS simulator
        ├── Subsystem-SS02: IMU
        │   └── Verification V-002: Analysis of drift rate
        └── Subsystem-SS03: Kalman filter
            └── Verification V-003: Monte Carlo analysis
```

### Implementation

```c
RequirementsMatrix *rm = arch_requirements_matrix_create();

// Add requirements
arch_rm_add_requirement(rm, "REQ-001", "Position accuracy ≤ 10m CEP",
                        RM_CRITICAL);
arch_rm_add_requirement(rm, "REQ-002", "Update rate ≥ 100 Hz",
                        RM_HIGH);
arch_rm_add_requirement(rm, "REQ-003", "Power ≤ 5W",
                        RM_MEDIUM);

// Add verification events
arch_rm_add_verification(rm, "V-001", "GPS simulator test", VM_TEST, "REQ-001");
arch_rm_add_verification(rm, "V-002", "IMU drift analysis", VM_ANALYSIS, "REQ-001");

// Check coverage
CoverageReport cr = arch_rm_coverage(rm);
printf("Requirements: %d, Verified: %d, Unverified: %d\n",
       cr.total, cr.verified, cr.unverified);
for (int i = 0; i < cr.n_unverified; i++) {
    printf("WARNING: %s not verified\n", cr.unverified_ids[i]);
}
```

| Implementation | File | Key Operations |
|---|---|---|
| Requirements matrix | `architecture_model.c` | `arch_requirements_matrix_create`, `arch_rm_add_requirement` |
| Verification linkage | `architecture_model.c` | `arch_rm_add_verification` |
| Coverage analysis | `architecture_model.c` | `arch_rm_coverage` |
| Gap analysis | `architecture_model.c` | `arch_rm_gap_analysis` |

---

## 4. Interface Management

Interfaces are the boundaries between system components where information,
energy, or material is exchanged. Mismanaged interfaces are one of the most
common sources of system integration failures.

### Interface Types

| Type | Description | Example |
|---|---|---|
| **Physical** | Mechanical, thermal, electrical connections | Connector pinout, bolt pattern, heat flux |
| **Data/Logical** | Information exchange | API, message format, protocol, timing |
| **Environmental** | Interaction with external environment | EMI/EMC, vibration, temperature range |
| **Human** | Human-machine interaction | Display layout, control ergonomics |

### Interface Control Document (ICD) Structure

For each interface:
- Owner (which subsystem controls each side)
- Type, direction, format
- Timing, rate, latency constraints
- Error handling, fault tolerance
- Version, change history

### N^2 Diagram (Design Structure Matrix)

An N×N matrix showing interfaces between N subsystems. Data flows from row
subsystem to column subsystem.

```
        SS1   SS2   SS3   SS4
SS1      X     →     →     
SS2      ←     X           →
SS3      ←           X     
SS4            ←           X
```

Circular interfaces (SS1↔SS2) indicate coupling that may need attention.

```c
InterfaceManager *im = arch_interface_manager_create(4);

// Define subsystems
arch_im_add_subsystem(im, "SS1", "GPS Receiver");
arch_im_add_subsystem(im, "SS2", "IMU");
arch_im_add_subsystem(im, "SS3", "Navigation Computer");
arch_im_add_subsystem(im, "SS4", "Flight Controller");

// Define interfaces
arch_im_add_interface(im, "SS1", "SS3", IF_DATA,
    "Position/velocity NMEA messages, 10 Hz, RS-232");
arch_im_add_interface(im, "SS2", "SS3", IF_DATA,
    "Accel/gyro raw data, 1 kHz, SPI");
arch_im_add_interface(im, "SS3", "SS4", IF_DATA,
    "Navigation solution, 100 Hz, CAN bus");

// Check for issues
arch_interface_check(im);
// Output: SS4 has no outgoing interface — intentional or missing?
//         SS1↔SS3 is one-directional — fine
```

| Implementation | File | Key Operations |
|---|---|---|
| Interface manager | `architecture_model.c` | `arch_interface_manager_create` |
| N^2 diagram | `architecture_model.c` | `arch_im_n2_matrix` |
| Interface validation | `architecture_model.c` | `arch_interface_check` |

---

## 5. System Decomposition

System decomposition breaks a complex system into manageable subsystems,
guided by principles of high cohesion and low coupling.

### Decomposition Strategies

| Strategy | Description | Best For |
|---|---|---|
| **Functional** | Decompose by function (sensing, processing, actuation) | Well-understood domains |
| **Physical** | Decompose by physical location or assembly | Distributed systems |
| **Layered** | Decompose by abstraction level (hardware → OS → middleware → app) | Software-intensive systems |
| **Object-oriented** | Decompose around data/state ownership | Information systems |

### Design Structure Matrix (DSM) for Decomposition

The DSM reveals coupling between subsystems. The goal is a modular (block
diagonal) structure with minimal off-diagonal interactions.

```
Before reordering:        After reordering (modular):
  A B C D E                  A E C B D
A X   X                   A X X
B   X   X                 E X X
C X   X   X               C     X X
D   X   X                 B       X   X
E       X X               D         X X
```

### Hierarchical Decomposition

```
System Level 0: Satellite Bus
├── Level 1: Power Subsystem
│   ├── Level 2: Solar Arrays
│   ├── Level 2: Battery
│   └── Level 2: Power Distribution Unit
├── Level 1: Attitude Control Subsystem
│   ├── Level 2: Reaction Wheels
│   ├── Level 2: Magnetorquers
│   └── Level 2: Star Tracker
└── Level 1: Communications Subsystem
    ├── Level 2: Transmitter
    ├── Level 2: Receiver
    └── Level 2: Antenna
```

```c
DecompositionTree *dt = arch_decompose_create("Satellite");

// Level 1
DecompNode *power = arch_decomp_add_child(dt->root, "Power Subsystem");
DecompNode *adcs  = arch_decomp_add_child(dt->root, "Attitude Control");
DecompNode *comms = arch_decomp_add_child(dt->root, "Communications");

// Level 2
arch_decomp_add_child(power, "Solar Arrays");
arch_decomp_add_child(power, "Battery");
arch_decomp_add_child(power, "Power Distribution");

arch_decomp_add_child(adcs, "Reaction Wheels");
arch_decomp_add_child(adcs, "Magnetorquers");
arch_decomp_add_child(adcs, "Star Tracker");

arch_decomp_add_child(comms, "Transmitter");
arch_decomp_add_child(comms, "Receiver");
arch_decomp_add_child(comms, "Antenna");

// Allocate requirements to subsystems
arch_decomp_allocate(dt, "REQ-001", adcs->children[0]); // Reaction Wheels
arch_decomp_allocate(dt, "REQ-003", power->children[1]); // Battery

arch_decomp_print(dt, 0);
```

| Implementation | File | Key Operations |
|---|---|---|
| Hierarchical decomposition | `architecture_model.c` | `arch_decompose_create`, `arch_decomp_add_child` |
| Requirement allocation | `architecture_model.c` | `arch_decomp_allocate` |
| DSM matrix computation | `architecture_model.c` | `arch_decomp_dsm` computes coupling matrix |
| Modularity metric | `architecture_model.c` | `arch_decomp_modularity` — ratio of internal to external interfaces |

---

## Summary Table

| Concept | Module | Key Functions |
|---|---|---|
| V-model lifecycle | `architecture_model.h` | `arch_vmodel_create`, `arch_vmodel_trace` |
| Trade space, Pareto frontier | `tradespace.h` | `ts_create`, `ts_pareto_frontier`, `ts_sensitivity` |
| Requirements verification matrix | `architecture_model.h` | `arch_requirements_matrix_create`, `arch_rm_coverage` |
| Interface management, N^2 diagram | `architecture_model.h` | `arch_interface_manager_create`, `arch_interface_check` |
| System decomposition, DSM | `architecture_model.h` | `arch_decompose_create`, `arch_decomp_dsm` |
