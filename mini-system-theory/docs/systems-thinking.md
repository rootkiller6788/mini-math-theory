# Systems Thinking

Core principles from Donella Meadows (*Thinking in Systems*) and Jay Forrester
(industrial dynamics, system dynamics), mapped to C implementations in
`mini-system-theory`.

---

## 1. Stocks and Flows

**Principle:** Stocks are accumulations — the memory of the system. Flows are
rates that change stocks over time. A stock is the integral of its net flow.

```
d(Stock)/dt = Inflow(t) - Outflow(t)
```

In a bathtub: water level (stock) = integral of (faucet inflow - drain outflow).

Stocks create inertia and delays. They decouple inflows from outflows, allowing
disequilibrium. Understanding stock-flow structure reveals why systems respond
slowly to interventions.

| Implementation | File | Key Operations |
|---|---|---|
| Stock-and-flow structure | `system_dynamics.c` | `sd_stock_flow_create` creates named stocks with initial values |
| Flow computation | `system_dynamics.c` | Rate equations defined as functions of current stocks |
| Euler integration | `system_dynamics.c` | `sd_simulate` runs forward Euler with configurable time step |
| Runge-Kutta 4 | `system_dynamics.c` | Higher-order integration for stiff systems |
| Bathtub model | `examples/resource_control_demo.c` | Inflow/outflow with feedback control |

**Example in C:**

```c
SDModel *m = sd_create("bathtub", 0.01); // dt = 0.01
sd_stock_flow_add(m, "water_level", 0.0);  // initially empty

// Flows as function pointers
double inflow_fn(SDModel *m)  { return 5.0; }      // constant faucet
double outflow_fn(SDModel *m) { return sd_get_stock(m, "water_level") * 0.1; }

sd_flow_add(m, "faucet", "water_level", inflow_fn);
sd_flow_add(m, "drain",  "water_level", outflow_fn, SD_FLOW_OUT);

sd_simulate(m, 100.0); // simulate 100 time units
```

---

## 2. Feedback Loops (Reinforcing vs Balancing)

**Principle:** Feedback loops are closed chains of causal connections from a
stock, through a set of decisions/rules, back to a flow that changes the stock.

- **Reinforcing (R) / Positive loops:** Amplify change. Compound interest,
  population growth, nuclear chain reaction. Exponential growth or collapse.
- **Balancing (B) / Negative loops:** Counteract change, seeking equilibrium.
  Thermostat, predator-prey, inventory control. Goal-seeking behavior.

| Implementation | File | Key Operations |
|---|---|---|
| Causal loop graph | `causal_loop.c` | `cl_create` builds a directed graph with signed edges |
| Loop detection | `causal_loop.c` | `cl_find_loops` finds all cycles using depth-first search |
| Polarity analysis | `causal_loop.c` | `cl_analyze_polarity`: even number of negative links = reinforcing, odd = balancing |
| Loop dominance | `causal_loop.c` | `cl_dominant_loop` identifies which loop drives behavior at current state |
| Combined simulation | `system_dynamics.c` + `causal_loop.c` | Stock-flows with loop identification overlay |

**Example in C:**

```c
CausalGraph *g = cl_create("population_model");
cl_add_node(g, "population", CL_STOCK);
cl_add_node(g, "births",     CL_FLOW);
cl_add_node(g, "deaths",     CL_FLOW);
cl_add_node(g, "food_per_capita", CL_AUXILIARY);

// Reinforcing loop: population → births → population (R)
cl_add_link(g, "population", "births",     CL_POSITIVE);
cl_add_link(g, "births",     "population", CL_POSITIVE);

// Balancing loop: population → food_per_capita → deaths → population (B)
cl_add_link(g, "population", "food_per_capita", CL_NEGATIVE);
cl_add_link(g, "food_per_capita", "deaths",     CL_NEGATIVE);
cl_add_link(g, "deaths",     "population", CL_NEGATIVE);

cl_analyze_polarity(g);
// Output: Loop 1 (births) → Reinforcing (R)
//         Loop 2 (food)   → Balancing (B)
```

---

## 3. Delays

**Principle:** Delays are ubiquitous in systems and are a major source of
instability and oscillation. A delay between taking action and seeing its
effect causes overshoot and oscillation. There are three types:

1. **Material delays:** Physical transit time (pipeline delay)
2. **Information delays:** Measurement and reporting lag
3. **Decision delays:** Time to perceive, decide, and act

| Implementation | File | Key Operations |
|---|---|---|
| Pipeline delay (fixed order) | `system_dynamics.c` | `sd_add_delay` stores a FIFO queue of delayed values |
| First-order exponential delay | `system_dynamics.c` | `sd_first_order` with time constant tau |
| Third-order delay (Erlang) | `system_dynamics.c` | `sd_nth_order_delay` approximates pipeline with n cascaded first-order lags |
| Delay as transfer function | `control_loop.c` | Padé approximation: e^(-sT), first/second-order Pade |

**Example in C:**

```c
// Pipeline delay: output equals input after delay_time
DelayPipeline *d = sd_delay_pipeline_create(20, 0.1); // 20 slots, dt=0.1
for (int i = 0; i < 100; i++) {
    sd_delay_update(d, input[i]);
    double output = sd_delay_output(d);
    // output[i] == input[i-20] (with 20-step delay)
}

// Third-order material delay (smoother than pipeline)
double output = sd_nth_order_delay(3, delay_time, input, dt);
```

---

## 4. Non-linearity

**Principle:** Most real-world relationships are non-linear. Linear thinking
assumes proportionality (doubling cause doubles effect), but non-linearities
produce thresholds, saturation, diminishing returns, and tipping points.

| Implementation | File | Key Operations |
|---|---|---|
| Table functions (lookup) | `system_dynamics.c` | `sd_table_function` for arbitrary non-linear relationships |
| S-shaped growth (logistic) | `system_dynamics.c` | `sd_logistic` with carrying capacity |
| Saturation (arctan, tanh) | `system_dynamics.c` | `sd_saturation` clamps output between bounds |
| Threshold effects | `system_dynamics.c` | Step functions, conditional flow rates |
| Non-linear state-space | `state_space.c` | `ss_create_nonlinear` with Jacobian linearization |

**Example in C:**

```c
// Non-linear table function: effect of inventory coverage on ordering
double coverage_points[] = {0.5, 1.0, 1.5, 2.0, 3.0};
double effect_points[]   = {1.5, 1.0, 0.6, 0.3, 0.0};
TableFunction *tf = sd_table_create(coverage_points, effect_points, 5);

double order_effect = sd_table_lookup(tf, current_coverage);
// Non-proportional: coverage of 1.5 doesn't give 1.5x effect
```

---

## 5. Emergence

**Principle:** System-level behavior emerges from the interactions of parts and
cannot be predicted by studying parts in isolation. "The whole is more than the
sum of its parts" (Aristotle). Emergence arises from feedback, non-linearity,
and network effects.

| Implementation | File | Key Operations |
|---|---|---|
| Multi-loop simulation | `system_dynamics.c` | Multiple interacting stock-flow structures |
| Agent-based emergence | `demos/mini-complex-system-simulator/` | Individual agents with simple rules → collective behavior |
| Sensitivity to initial conditions | `stability.c` | `stability_lyapunov_exponent` measures chaos |
| Phase space plotting | `state_space.c` | `ss_phase_portrait` visualizes attractors |
| Limit cycles | `stability.c` | `stability_detect_limit_cycle` |

**Example in C:**

```c
// Emergence: simple economy model
// 3 simple rules → complex macro behavior (business cycles, growth)
Stock stocks[] = {
    {"capital", 100.0},
    {"labor",   50.0},
    {"goods",   0.0}
};
// Capital → production → goods → sales → capital (reinforcing)
// Capital → depreciation → capital (balancing)
// Goods → price → demand → sales (balancing)
// Together these produce emergent GDP oscillations
sd_simulate(economy, 1000.0);
// Output shows cycles that no single equation describes
```

---

## 6. Leverage Points

**Principle:** Places to intervene in a system, ranked by effectiveness
(Meadows, 1999):

| Rank (increasing effectiveness) | Leverage Point | Implementation |
|---|---|---|
| 12. Constants, parameters, numbers | Tuning PID gains | `feedback.c` via `pid_create(Kp, Ki, Kd)` |
| 11. Size of buffers and stocks | Queue capacity | `queue_model.c` via `queue_mmc_create(capacity)` |
| 10. Material flows and nodes | Rate changes | `system_dynamics.c` flow function pointers |
| 9. Delays (length relative to system change rate) | Delay time | `system_dynamics.c` `sd_add_delay` |
| 8. Balancing feedback loops | Loop strength | `causal_loop.c` link weight analysis |
| 7. Reinforcing feedback loops | Growth rate | `causal_loop.c` `cl_find_loops` identification |
| 6. Information flows | Measurement points | `causal_loop.c` information link analysis |
| 5. Rules of the system | PID structure, queue discipline | `feedback.c` control law, `queue_model.c` scheduling |
| 4. Self-organization | Adaptive control | `feedback.c` auto-tuning, `queue_model.c` adaptive routing |
| 3. Goals of the system | Setpoint | `feedback.c` reference input in `pid_update` |
| 2. Mindset/paradigm | System boundary | `causal_loop.c` graph scope, what's endogenous vs exogenous |
| 1. Transcend paradigms | Model selection | Choosing state-space vs stock-flow vs queue representation |

```c
CLeverageAnalysis *la = cl_leverage_points(graph);
for (int i = 0; i < la->n_points; i++) {
    printf("Rank %d: %s (effectiveness: %.2f)\n",
           la->points[i].rank,
           la->points[i].name,
           la->points[i].sensitivity);
}
```

---

## Summary Table

| Principle | Primary Module | Key Concept |
|---|---|---|
| Stocks and flows | `system_dynamics.h` | Accumulation, inertia, Euler/RK4 integration |
| Feedback loops | `causal_loop.h` | Signed digraphs, loop polarity, reinforcing vs balancing |
| Delays | `system_dynamics.h` | Pipeline, exponential, Erlang delays |
| Non-linearity | `system_dynamics.h` | Table functions, saturation, thresholds |
| Emergence | `system_dynamics.h` | Multi-loop interaction, phase portraits |
| Leverage points | `causal_loop.h` | Sensitivity analysis, intervention ranking |
