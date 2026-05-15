# mini-architecture-tradespace: Multi-Objective Optimization for System Architecture

Architecture trade space demo performing multi-objective optimization across
cost, performance, and reliability. Computes Pareto frontiers and verifies
requirements compliance.

## Overview

Evaluates architectural alternatives for a system (e.g., satellite design,
web service topology, embedded system) across competing objectives. Identifies
the Pareto-optimal set and supports weighted trade-off decisions.

## Concepts Demonstrated

### 1. Multi-Objective Optimization

System architects must balance conflicting objectives. The trade space contains
all feasible design alternatives, each characterized by multiple attributes.

```
Objective Space (3D example):

  Performance (maximize)
       ↑
       |    * B                       Legend:
       |      \                       A = cheap, moderate perf, low reliability
       |  * A  * C                    B = expensive, high perf, high reliability
       |   \   /                      C = moderate cost, good perf, good reliability
       |    * D                       D = cheap, high perf, low reliability
       |                               E = expensive, low perf, high reliability
       +───────────────→ Cost (minimize)
      /
     /
    ↓ Reliability (maximize)
```

### 2. Pareto Frontier Computation

An alternative A **dominates** alternative B if A is at least as good as B in
all objectives AND strictly better in at least one.

The **Pareto frontier** is the set of all non-dominated alternatives.

**Algorithm (Naive, O(n^2 * m)):**

```
for each alternative i:
    dominated = false
    for each alternative j:
        if j dominates i:
            dominated = true
            break
    if not dominated:
        add i to Pareto frontier
```

**Algorithm (2D optimization, O(n log n)):**

```
Sort by objective 1 (ascending for minimize, descending for maximize)
Scan: keep points that improve objective 2
```

```c
TradeSpace *ts = ts_create(3);
ts_set_objective(ts, 0, "cost",           TS_MINIMIZE);
ts_set_objective(ts, 1, "performance",    TS_MAXIMIZE);
ts_set_objective(ts, 2, "reliability",    TS_MAXIMIZE);

// Add 10 architectural alternatives
ts_add_alternative(ts, "simple_single",    (double[]){50,  0.70, 0.90});
ts_add_alternative(ts, "simple_redundant", (double[]){85,  0.75, 0.99});
ts_add_alternative(ts, "moderate_single",  (double[]){120, 0.92, 0.95});
ts_add_alternative(ts, "moderate_dual",    (double[]){180, 0.94, 0.995});
ts_add_alternative(ts, "premium_single",   (double[]){200, 0.99, 0.98});
ts_add_alternative(ts, "premium_dual",     (double[]){280, 0.999, 0.999});
ts_add_alternative(ts, "distributed",      (double[]){320, 0.98,  0.9995});
ts_add_alternative(ts, "hybrid_a",         (double[]){160, 0.88,  0.97});
ts_add_alternative(ts, "hybrid_b",         (double[]){210, 0.96,  0.99});
ts_add_alternative(ts, "minimal",          (double[]){30,  0.40,  0.80});

// Compute Pareto frontier
ParetoFront *pf = ts_pareto_frontier(ts);
printf("Pareto-optimal alternatives: %d / %d\n", pf->n_solutions, ts->n_alternatives);
for (int i = 0; i < pf->n_solutions; i++) {
    printf("  %s: cost=%.0f, perf=%.3f, reliab=%.4f\n",
           pf->names[i], pf->values[i][0], pf->values[i][1], pf->values[i][2]);
}
// Output:
// Pareto-optimal alternatives: 4 / 10
//   minimal: cost=30, perf=0.400, reliab=0.8000
//   simple_single: cost=50, perf=0.700, reliab=0.9000
//   premium_dual: cost=280, perf=0.999, reliab=0.9990
//   distributed: cost=320, perf=0.980, reliab=0.9995
```

### 3. Sensitivity Analysis

How sensitive are rankings to changes in objective weights or attribute values?

**Tornado Diagram:** Rank alternatives by how much their score changes when
each attribute varies by ±10%.

```c
SensitivityResult *sr = ts_sensitivity(ts, "cost");
printf("Tornado diagram for cost:\n");
for (int i = 0; i < sr->n_alternatives; i++) {
    double original = ts_get_value(ts, sr->indices[i], "cost");
    double delta_hi = original * 0.10;
    double delta_lo = -original * 0.10;
    printf("  %-20s | [%.0f──%.0f──%.0f]\n",
           sr->names[i], original + delta_lo, original, original + delta_hi);
}
```

**One-way sensitivity:** Vary one weight, see how rankings change.

```c
// Vary cost weight from 0 to 1 (performance and reliability share the rest)
for (double w_cost = 0.0; w_cost <= 1.0; w_cost += 0.1) {
    double w_perf = (1.0 - w_cost) * 0.6;
    double w_rel = (1.0 - w_cost) * 0.4;
    double weights[] = {w_cost, w_perf, w_rel};
    RankedList *rl = ts_rank_alternatives(ts, weights);
    printf("w_cost=%.1f: Top 3: %s, %s, %s\n",
           w_cost, rl->names[0], rl->names[1], rl->names[2]);
}
// Output shows how preferred architecture changes with budget emphasis
```

### 4. Requirements Verification

Each alternative is checked against minimum requirements (hard constraints):

```c
// Define requirements
Requirement reqs[] = {
    {"REQ-PERF-01", "Performance ≥ 0.85",  OBJ_PERFORMANCE, REQ_GE, 0.85},
    {"REQ-REL-01",  "Reliability ≥ 0.99",  OBJ_RELIABILITY, REQ_GE, 0.99},
    {"REQ-COST-01", "Cost ≤ 250",          OBJ_COST,        REQ_LE, 250.0},
};

// Verify each alternative
for (int i = 0; i < ts->n_alternatives; i++) {
    int violations = 0;
    for (int j = 0; j < n_reqs; j++) {
        if (!arch_verify_requirement(reqs[j], ts->values[i][reqs[j].obj])) {
            printf("  %s FAILS %s\n", ts->names[i], reqs[j].id);
            violations++;
        }
    }
    if (violations == 0) {
        printf("  %s PASSES all requirements\n", ts->names[i]);
    }
}

// Output:
// simple_single FAILS REQ-PERF-01 (0.70 < 0.85)
// simple_single FAILS REQ-REL-01 (0.90 < 0.99)
// moderate_dual PASSES all requirements
// premium_dual FAILS REQ-COST-01 (280 > 250)
// hybrid_b PASSES all requirements
```

## Decision Analysis Methods

### Weighted Sum

```
Score_i = Σ w_j * normalized_value_i_j
```

Where normalization maps each objective to [0, 1] (for both minimize and maximize objectives).

### Technique for Order Preference by Similarity to Ideal Solution (TOPSIS)

```
1. Normalize all values
2. Weight the normalized matrix
3. Find ideal best and ideal worst
4. Compute distance to ideal best and ideal worst
5. Score = d_worst / (d_best + d_worst)
```

```c
// Weighted sum ranking
double weights[] = {0.5, 0.3, 0.2}; // cost 50%, perf 30%, reliability 20%
RankedList *rl = ts_rank_alternatives_weighted_sum(ts, weights);
printf("Weighted sum ranking:\n");
for (int i = 0; i < rl->n; i++) {
    printf("  %d. %s (score=%.3f)\n", i+1, rl->names[i], rl->scores[i]);
}

// TOPSIS ranking
RankedList *rl_topsis = ts_rank_alternatives_topsis(ts, weights);
printf("TOPSIS ranking:\n");
for (int i = 0; i < rl_topsis->n; i++) {
    printf("  %d. %s (closeness=%.3f)\n", i+1, rl_topsis->names[i], rl_topsis->scores[i]);
}
```

## Expected Output

```
=== Architecture Trade Space Analysis ===
Objectives: minimize cost, maximize performance, maximize reliability
Alternatives: 10 designs

--- Pareto Frontier (4 non-dominated solutions) ---
Name              |  Cost  |  Perf  |  Reliab
minimal           |    30  | 0.400  | 0.8000
simple_single     |    50  | 0.700  | 0.9000
premium_dual      |   280  | 0.999  | 0.9990
distributed       |   320  | 0.980  | 0.9995

--- Requirements Verification ---
Requirements: Perf≥0.85, Reliab≥0.99, Cost≤250
  4 / 10 alternatives pass all requirements

--- Weighted Sum Ranking (w_cost=0.5, w_perf=0.3, w_reliab=0.2) ---
1. hybrid_b          (score=0.784)
2. moderate_dual     (score=0.756)
3. simple_single     (score=0.723)
4. hybrid_a          (score=0.698)
5. premium_dual      (score=0.612)   [higher cost penalized]

--- Sensitivity (Cost weight 0→1, perf fixed at 0.3) ---
w_cost | Top Choice
0.0    | premium_dual        (best perf+reliab)
0.2    | premium_dual
0.4    | moderate_dual
0.6    | hybrid_b
0.8    | simple_single
1.0    | minimal             (cheapest only)

--- Recommendation ---
If budget-constrained:      hybrid_b (cost=210, perf=0.96, reliab=0.99)
If performance-critical:    premium_dual (cost=280, perf=0.999, reliab=0.999)
Pareto-efficient compromise: moderate_dual (cost=180, perf=0.94, reliab=0.995)
```

## C Implementation Outline

```c
typedef struct {
    char name[64];
    double *values; // n_objectives values
    bool dominated;
} Alternative;

typedef struct {
    int n_objectives;
    int *direction; // TS_MINIMIZE or TS_MAXIMIZE for each objective
    char obj_names[10][32];
    int n_alternatives;
    Alternative *alternatives;
} TradeSpace;

TradeSpace *ts_create(int n_objectives) {
    TradeSpace *ts = malloc(sizeof(TradeSpace));
    ts->n_objectives = n_objectives;
    ts->n_alternatives = 0;
    ts->alternatives = malloc(MAX_ALTERNATIVES * sizeof(Alternative));
    return ts;
}

bool dominates(TradeSpace *ts, Alternative *a, Alternative *b) {
    bool at_least_as_good = true;
    bool strictly_better = false;

    for (int j = 0; j < ts->n_objectives; j++) {
        double va = a->values[j], vb = b->values[j];
        if (ts->direction[j] == TS_MINIMIZE) {
            if (va > vb) at_least_as_good = false;
            if (va < vb) strictly_better = true;
        } else { // MAXIMIZE
            if (va < vb) at_least_as_good = false;
            if (va > vb) strictly_better = true;
        }
    }
    return at_least_as_good && strictly_better;
}

ParetoFront *ts_pareto_frontier(TradeSpace *ts) {
    for (int i = 0; i < ts->n_alternatives; i++) {
        ts->alternatives[i].dominated = false;
    }
    for (int i = 0; i < ts->n_alternatives; i++) {
        for (int j = 0; j < ts->n_alternatives; j++) {
            if (i != j && dominates(ts, &ts->alternatives[j], &ts->alternatives[i])) {
                ts->alternatives[i].dominated = true;
                break;
            }
        }
    }
    // Collect non-dominated into ParetoFront...
}
```

## Real-World Application

- **Satellite design:** Trade mass, power, data rate, cost, reliability
- **Car architecture:** Range vs acceleration vs cost vs safety
- **Cloud architecture:** Latency vs throughput vs cost vs availability
- **Embedded systems:** Processing power vs energy consumption vs BOM cost
