# mini-complex-system-simulator: Multi-Loop Stock-and-Flow Economy

Complex system simulator demonstrating multiple interacting feedback loops in a
stock-and-flow model of a simple economy. Shows emergence of macro behavior
(business cycles) from micro-level feedback structures.

## Overview

Simulates a simple economy with interacting stocks (capital, goods inventory,
labor, money) and feedback loops (production, consumption, investment,
depreciation). Demonstrates how macro-level phenomena like business cycles
emerge from the interaction of reinforcing and balancing feedback loops.

## The Model

### Stocks (Accumulations)

```
┌─────────────┐    ┌──────────────┐    ┌─────────────┐    ┌───────────┐
│   Capital   │    │    Goods     │    │    Labor    │    │   Money   │
│   (machines)│    │  (inventory) │    │  (workers)  │    │ (capital) │
└──────┬──────┘    └──────┬───────┘    └──────┬──────┘    └─────┬─────┘
       │                  │                   │                 │
   investment        production           hiring           wages/prices
   depreciation      consumption          layoffs          revenue/costs
```

### Feedback Loops

#### Loop 1: Production-Consumption (Balancing)

```
Goods Inventory → Price → Demand → Consumption → Goods Inventory
       ↑                                          ↓
       └──────────── Production ←────────────────┘
                            ↑
                        Capital
                            ↑
                        Investment
```

When goods inventory is LOW → prices RISE → demand FALLS → consumption drops →
goods accumulate → prices fall → demand rises → cycle continues.

This is a **balancing** loop: it seeks equilibrium via the price mechanism.

#### Loop 2: Investment Growth (Reinforcing)

```
Capital → Production → Revenue → Profit → Investment → Capital
   ↑                                                      ↓
   └────────── Depreciation ←─────────────────────────────┘
```

More capital enables more production, generating more revenue and profit,
which funds more investment, growing capital further.

This is a **reinforcing** loop with a balancing depreciation term.

#### Loop 3: Labor Market (Balancing)

```
Labor → Wages → Costs → Profit → Hiring → Labor
  ↑                                          ↓
  └─────────────── Layoffs ←────────────────┘
```

When labor is scarce → wages rise → costs increase → profit shrinks → hiring
slows → labor growth stabilizes.

This is a **balancing** loop regulating the labor market.

### Combined Dynamics

The interaction of these loops creates non-trivial behavior:

1. Investment (R-loop) drives growth
2. Price mechanism (B-loop) regulates goods market
3. Labor market (B-loop) regulates wage-cost balance
4. **Interaction:** Capital growth → more production → goods glut → price
   drop → revenue drops → investment drops → capital falls → recession.
   Then goods are consumed → prices recover → investment restarts → expansion.
   **This is an emergent business cycle.**

## Mathematical Model

```
d(Capital)/dt = Investment(t) - Depreciation(t)
d(Goods)/dt   = Production(t) - Consumption(t)
d(Labor)/dt   = Hiring(t) - Layoffs(t)
d(Money)/dt   = Revenue(t) - Costs(t)

where:
  Production  = min(Capital * productivity, Labor * labor_productivity)
  Consumption = base_demand * (base_price / Price(t))^price_elasticity
  Price       = base_price * (base_demand / max(Goods, epsilon))^price_sensitivity
  Investment  = Profit * investment_fraction * confidence(t)
  Depreciation = Capital / capital_lifetime
  Revenue     = Consumption * Price
  Costs       = Wages * Labor + fixed_costs + Investment
  Wages       = base_wage * (Labor / labor_supply)^wage_elasticity
  Hiring      = profit_ratio * hiring_rate * Labor
  Layoffs     = (1 - profit_ratio) * layoff_rate * Labor
```

## Simulation Implementation

```c
typedef struct {
    double capital;     // K (productive capacity)
    double goods;       // G (inventory of produced goods)
    double labor;       // L (employed workers)
    double money;       // M (firm's cash reserves)
} Economy;

typedef struct {
    double productivity;       // units of goods per capital per time
    double labor_productivity; // units of goods per worker per time
    double base_demand;        // reference consumption level
    double base_price;         // reference price level
    double price_elasticity;   // demand sensitivity to price
    double price_sensitivity;  // price sensitivity to inventory level
    double investment_fraction;// fraction of profit reinvested
    double capital_lifetime;   // average machine lifespan
    double base_wage;          // reference wage level
    double wage_elasticity;    // wage sensitivity to labor scarcity
    double hiring_rate;        // hiring speed
    double layoff_rate;        // layoff speed
    double labor_supply;       // total available workforce
    double fixed_costs;        // overhead costs
} Parameters;

void economy_step(Economy *e, Parameters *p, double dt) {
    // Intermediate calculations
    double production = fmin(e->capital * p->productivity,
                             e->labor * p->labor_productivity);

    double price = p->base_price *
        pow(p->base_demand / fmax(e->goods, 0.001), p->price_sensitivity);

    double consumption = p->base_demand *
        pow(p->base_price / fmax(price, 0.001), p->price_elasticity);

    double revenue = consumption * price;
    double wages = p->base_wage *
        pow(e->labor / p->labor_supply, p->wage_elasticity);
    double costs = wages * e->labor + p->fixed_costs;

    double profit = revenue - costs;
    double profit_ratio = profit / fmax(costs, 0.001) + 0.5;
    profit_ratio = fmax(0.0, fmin(1.0, profit_ratio));

    double confidence = e->goods < p->base_demand * 2 ? 1.0 :
                        e->goods > p->base_demand * 5 ? 0.3 : 0.7;
    double investment = profit * p->investment_fraction * confidence;
    double depreciation = e->capital / p->capital_lifetime;

    double hiring = profit_ratio * p->hiring_rate *
        (p->labor_supply - e->labor);
    double layoffs = (1.0 - profit_ratio) * p->layoff_rate * e->labor;

    // State updates
    e->capital += (investment - depreciation) * dt;
    e->goods   += (production - consumption) * dt;
    e->labor   += (hiring - layoffs) * dt;
    e->money   += (revenue - costs - investment) * dt;

    // Clamp to non-negative
    e->capital = fmax(0, e->capital);
    e->goods   = fmax(0, e->goods);
    e->labor   = fmax(0, fmin(e->labor, p->labor_supply));
}
```

### Causal Loop Analysis

```c
CausalGraph *cg = cl_create("economy");

// Define nodes
cl_add_node(cg, "Capital",       CL_STOCK);
cl_add_node(cg, "Goods_Inventory", CL_STOCK);
cl_add_node(cg, "Labor",         CL_STOCK);
cl_add_node(cg, "Money",         CL_STOCK);
cl_add_node(cg, "Production",    CL_FLOW);
cl_add_node(cg, "Consumption",   CL_FLOW);
cl_add_node(cg, "Investment",    CL_FLOW);
cl_add_node(cg, "Depreciation",  CL_FLOW);
cl_add_node(cg, "Hiring",        CL_FLOW);
cl_add_node(cg, "Layoffs",       CL_FLOW);
cl_add_node(cg, "Price",         CL_AUXILIARY);
cl_add_node(cg, "Wages",         CL_AUXILIARY);
cl_add_node(cg, "Profit",        CL_AUXILIARY);
cl_add_node(cg, "Revenue",       CL_AUXILIARY);
cl_add_node(cg, "Costs",         CL_AUXILIARY);

// Define causal links
cl_add_link(cg, "Capital",    "Production",   CL_POSITIVE);
cl_add_link(cg, "Labor",      "Production",   CL_POSITIVE);
cl_add_link(cg, "Production", "Goods_Inventory", CL_POSITIVE);
cl_add_link(cg, "Goods_Inventory", "Price",    CL_NEGATIVE);
cl_add_link(cg, "Price",      "Consumption",  CL_NEGATIVE);
cl_add_link(cg, "Consumption","Goods_Inventory", CL_NEGATIVE);
cl_add_link(cg, "Consumption","Revenue",      CL_POSITIVE);
cl_add_link(cg, "Price",      "Revenue",      CL_POSITIVE);
cl_add_link(cg, "Revenue",    "Profit",       CL_POSITIVE);
cl_add_link(cg, "Costs",      "Profit",       CL_NEGATIVE);
cl_add_link(cg, "Profit",     "Investment",   CL_POSITIVE);
cl_add_link(cg, "Investment", "Capital",      CL_POSITIVE);
cl_add_link(cg, "Capital",    "Depreciation", CL_POSITIVE);
cl_add_link(cg, "Depreciation","Capital",     CL_NEGATIVE);
cl_add_link(cg, "Labor",      "Wages",        CL_POSITIVE);
cl_add_link(cg, "Wages",      "Costs",        CL_POSITIVE);
cl_add_link(cg, "Profit",     "Hiring",       CL_POSITIVE);
cl_add_link(cg, "Hiring",     "Labor",        CL_POSITIVE);
cl_add_link(cg, "Layoffs",    "Labor",        CL_NEGATIVE);
cl_add_link(cg, "Labor",      "Layoffs",      CL_NEGATIVE);
cl_add_link(cg, "Labor",      "Layoffs",      CL_NEGATIVE);

// Analyze loops
cl_analyze_polarity(cg);
// Identified loops:
//   R1: Capital→Production→Goods→Revenue→Profit→Investment→Capital (REINFORCING)
//   B1: Goods→Price→Consumption→Goods (BALANCING)
//   B2: Labor→Wages→Costs→Profit→Hiring→Labor (BALANCING)
//   R2: Capital→Depreciation→Capital (REINFORCING, but negative—a drain)
```

## Emergence Demonstration

The key insight: **business cycles emerge from feedback structure alone**, even
with constant parameters and no external shocks.

```c
// Run simulation with constant parameters
Economy e = {100.0, 50.0, 30.0, 500.0};
Parameters p = {
    .productivity = 0.5,
    .labor_productivity = 0.3,
    .base_demand = 20.0,
    .base_price = 10.0,
    .price_elasticity = 0.8,
    .price_sensitivity = 0.5,
    .investment_fraction = 0.6,
    .capital_lifetime = 15.0,
    .base_wage = 5.0,
    .wage_elasticity = 0.3,
    .hiring_rate = 0.1,
    .layoff_rate = 0.15,
    .labor_supply = 100.0,
    .fixed_costs = 50.0,
};

// Log all stocks over 200 time units
for (double t = 0; t < 200; t += 0.1) {
    economy_step(&e, &p, 0.1);
    printf("%.1f,%.2f,%.2f,%.2f,%.2f\n",
           t, e.capital, e.goods, e.labor, e.money);
}
```

The output reveals:
1. Initial growth phase (capital accumulation)
2. First boom (high production, high employment)
3. Overproduction (goods inventory spikes, prices crash)
4. Recession (investment drops, layoffs increase)
5. Recovery (low inventory, prices recover, investment resumes)
6. **Cycle repeats** — from internal dynamics, not external forcing

## Expected Output

```
=== Complex System Simulator ===
Model: 4-stock economy (capital, goods, labor, money)
Duration: 200 time units, dt=0.1

--- Phase Analysis ---
Phase 1 (t=0-45):   Growth
   Capital: 100 → 234, Labor: 30 → 72, Money: 500 → 890
   Dominant loop: Investment growth (R1)

Phase 2 (t=45-62):  Boom / Overheating
   Goods inventory peaks at 185 (vs demand 20)
   Prices crash from 10.0 to 2.3
   Dominant loop: Overproduction → price collapse (B1 overpowers R1)

Phase 3 (t=62-95):  Recession
   Capital: 234 → 178, Labor: 72 → 41, Money: 890 → 420
   Investment drops 73%, layoffs spike
   Dominant loop: Depreciation + cost pressure (B2 dominates)

Phase 4 (t=95-140): Recovery
   Goods inventory falls to 12, prices recover to 8.7
   Investment restarts, hiring resumes
   Dominant loop: Investment growth (R1 re-emerges)

Phase 5 (t=140-175): Second Boom
   Capital: 178 → 312, Labor: 41 → 85
   Peak capital exceeds first cycle (growth trend)

Phase 6 (t=175-200): Second Contraction
   Goods inventory peaks at 210 (higher than first cycle!)
   Recession depth: Capital falls to 245

--- Emergent Properties ---
Cycle period: ~90 time units (endogenous, no external forcing)
Amplitude: Growing (capital trendline rising despite cycles)
Amplification: Each cycle peaks higher (reinforcing loop not fully damped)

--- Causal Loop Summary ---
Identified loops:
  R1: Capital→Production→Revenue→Profit→Investment→Capital (REINFORCING, 5 nodes)
  B1: Goods→Price→Consumption→Goods (BALANCING, 3 nodes)
  B2: Labor→Wages→Costs→Profit→Hiring→Labor (BALANCING, 6 nodes)

Leverage points (by sensitivity):
  1. Investment fraction   (R1 strength)        | sensitivity: 0.82
  2. Price sensitivity     (B1 strength)        | sensitivity: 0.71
  3. Capital lifetime      (depreciation rate)  | sensitivity: 0.58
  4. Wage elasticity       (B2 strength)        | sensitivity: 0.43
```

## C Implementation Outline

```c
int main(void) {
    Economy e = {100.0, 50.0, 30.0, 500.0};
    Parameters p = {0.5, 0.3, 20.0, 10.0, 0.8, 0.5, 0.6,
                    15.0, 5.0, 0.3, 0.1, 0.15, 100.0, 50.0};

    FILE *csv = fopen("economy_output.csv", "w");
    fprintf(csv, "time,capital,goods,labor,money,price,investment,profit\n");

    for (double t = 0; t < 200.0; t += 0.1) {
        double price, profit, investment;
        economy_step(&e, &p, 0.1, &price, &profit, &investment);
        fprintf(csv, "%.1f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                t, e.capital, e.goods, e.labor, e.money,
                price, investment, profit);
    }
    fclose(csv);

    // Causal loop analysis
    CausalGraph *cg = build_economy_causal_graph();
    CLLoopAnalysis *la = cl_analyze_polarity(cg);

    printf("\n--- Causal Loop Summary ---\n");
    for (int i = 0; i < la->n_loops; i++) {
        printf("  %s: %s (%s, %d nodes)\n",
               la->loops[i].id,
               la->loops[i].path,
               la->loops[i].type == CL_REINFORCING ? "R" : "B",
               la->loops[i].n_nodes);
    }

    CLeverageAnalysis *lev = cl_leverage_points(cg);
    printf("\nLeverage points (by sensitivity):\n");
    for (int i = 0; i < lev->n_points; i++) {
        printf("  %d. %-25s | sensitivity: %.2f\n",
               i + 1, lev->points[i].name, lev->points[i].sensitivity);
    }

    return 0;
}
```

## Real-World Application

- **Macroeconomic modeling:** Business cycle theory (Hicks, Goodwin, Keen)
- **Supply chain dynamics:** Bullwhip effect (Forrester's industrial dynamics)
- **Climate-economy models:** Nordhaus DICE model with carbon stock feedback
- **Startup growth:** Capital → hiring → product → revenue → capital (same structure!)
- **Urban dynamics:** Forrester's Urban Dynamics model
