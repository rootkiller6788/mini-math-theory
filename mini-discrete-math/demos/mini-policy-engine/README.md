# Mini Policy Engine — Boolean Algebra Access Control

## Overview

A mini access-control policy engine that evaluates authorization rules using Boolean
algebra. Policies combine role checks, time-based constraints, and override flags
into Boolean expressions, which are compiled to gate-level circuits and evaluated
efficiently.

## Use Case

In a system where users have roles, operations must be authorized based on:
- The user's assigned role(s)
- Time-of-day restrictions
- Emergency override tokens

Policies are expressed as Boolean formulas like:

```
(role=admin AND time<18:00) OR emergency_override
```

## Architecture

```
User Request → Policy Parser → Boolean AST → Logic Circuit → ALLOW/DENY
```

- **Inputs**: Boolean variables representing `is_admin`, `is_hour_lt_18`, `emergency_override`
- **Policy**: A Boolean expression over these inputs
- **Evaluation**: Evaluate the circuit for the current combination of inputs
- **Output**: `true` = ALLOW, `false` = DENY

## Implementation Sketch

### Step 1: Define Policy Inputs

```c
typedef enum {
    INPUT_IS_ADMIN,
    INPUT_IS_MANAGER,
    INPUT_IS_ENGINEER,
    INPUT_HOUR_LT_18,
    INPUT_IS_BUSINESS_HOURS,
    INPUT_EMERGENCY_OVERRIDE,
    INPUT_IP_IN_ALLOWLIST,
    NUM_POLICY_INPUTS
} PolicyInput;

const char* input_names[] = {
    "is_admin", "is_manager", "is_engineer",
    "hour<18", "business_hours", "emergency", "ip_allowlist"
};
```

### Step 2: Build Policy as Boolean Expression

Use `formula_create()` and the logic AST from `logic.h`:

```c
// Policy: (is_admin AND hour<18) OR emergency_override
Formula* policy = formula_create();
policy->nvars = 3;
policy->var_names[0] = 'A';  // is_admin
policy->var_names[1] = 'T';  // hour<18
policy->var_names[2] = 'E';  // emergency

int a = formula_add_var(policy, 0);
int t = formula_add_var(policy, 1);
int e = formula_add_var(policy, 2);
int a_and_t = formula_add_and(policy, a, t);
int root = formula_add_or(policy, a_and_t, e);
```

### Step 3: Compile to Logic Circuit (for hardware/simulation)

Use the `boolalg.h` module to convert policies to gate-level circuits:

```c
LogicCircuit circuit;
circuit_init(&circuit, 3);  // 3 policy inputs

// a_and_t = (A AND T)
int g_and = circuit_add_gate(&circuit, GATE_AND, 0, 1);
// result = (a_and_t OR E)
int result = circuit_add_gate(&circuit, GATE_OR, circuit.ninputs + g_and, 2);
circuit.output_id = result;
```

### Step 4: Evaluate for a Request

```c
bool evaluate_policy(Formula* policy, int root,
                     bool is_admin, bool hour_lt_18, bool emergency) {
    bool assignment[] = { is_admin, hour_lt_18, emergency };
    return formula_eval(policy, root, assignment);
}

// OR using circuit:
bool evaluate_policy_circuit(LogicCircuit* c,
                              bool is_admin, bool hour_lt_18, bool emergency) {
    bool inputs[] = { is_admin, hour_lt_18, emergency };
    bool outputs[256];
    circuit_evaluate(c, inputs, outputs, 256);
    return outputs[c->output_id];
}
```

### Step 5: Verify Policy Correctness

Before deploying a policy, verify that it never accidentally allows or denies
in unexpected ways:

```c
void verify_policy(Formula* policy, int root) {
    // Is the policy a tautology? (always ALLOW — too permissive!)
    if (is_tautology(policy, root))
        printf("WARNING: Policy is TAUTOLOGY — grants access unconditionally!\n");

    // Is the policy a contradiction? (always DENY — useless!)
    if (is_contradiction(policy, root))
        printf("WARNING: Policy is CONTRADICTION — denies all access!\n");

    // Print truth table for audit
    printf("Policy truth table:\n");
    truth_table_print(policy, root);
}
```

## Sample Policies

### 1. Standard Role-Based Access
```
(role=admin) OR (role=manager AND business_hours)
```
- Admin always has access
- Manager has access only during business hours
- Engineer has no access (unless added explicitly)

### 2. Time-Windowed Access
```
(role=engineer) AND (hour>=9 AND hour<18)
```
Runs as: `E ∧ (H9 ∧ ¬H18)` — requires encoding two time bits.

### 3. Emergency Bypass
```
(role=admin AND hour<18) OR emergency_override
```
Even if the admin is outside hours, emergency override allows access.

### 4. Two-Person Rule
```
(role=admin AND supervisor_present) OR emergency_override
```
A sensitive operation requires both admin and supervisor (or emergency override).

### 5. IP-Restricted Admin
```
(role=admin) AND (ip_allowlist)
```
Admin can only operate from known IP ranges.

## Extensions

- **Nested policies**: Compose sub-policies with AND/OR, similar to AWS IAM policy evaluation.
- **Policy simplification**: Use `karnaugh_map_*()` from `boolalg.h` to minimize policy expressions — fewer gates = faster evaluation.
- **Audit log generation**: Emit the evaluation path (which clauses contributed to ALLOW/DENY) for debugging.
- **Temporal policies**: Extend time constraints with calendar-based rules using date comparison functions.

## Building

```bash
cd mini-discrete-math
gcc -I include -o bin/policy-engine \
    demos/mini-policy-engine/policy_engine.c \
    src/logic.c src/boolalg.c -lm
```

## Dependencies

- `logic.h` / `logic.c` — Boolean formula AST and evaluation
- `boolalg.h` / `boolalg.c` — Gate-level circuit simulation and Karnaugh map simplification
