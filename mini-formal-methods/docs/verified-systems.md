# Verified Systems in C

## What We Can (and Cannot) Verify

### Runtime Contract Checking
Preconditions, postconditions, and invariants checked at each function call.
- **Works for**: catching bugs during development and testing
- **Limitation**: not a proof — only checks specific executions

### Symbolic Execution
Explores ALL possible input values within a bounded range.
- **Works for**: small domains (e.g., 8-bit values, small arrays)
- **Limitation**: exponential in number of variables

### Bounded Model Checking (BMC)
Checks state machine properties up to a bound k.
- **Works for**: finite-state systems with known bound
- **Limitation**: doesn't prove "for all depths"

### What We CAN'T Do (in C alone)
- Full functional correctness proofs → needs Coq/Lean/Dafny
- Infinite-state verification → needs theorem provers
- Concurrent program verification → needs TLA+/PlusCal
- Full temporal logic (LTL/CTL) → needs model checkers (NuSMV, Spin)

## When to Use Each Tool

| Need | Our C Framework | External Tool |
|------|----------------|---------------|
| Quick sanity checks | `contracts.h` ✓ | |
| Loop invariant debugging | `invariants.h` ✓ | |
| Bounded exhaustive test | `sym_exec_explore` ✓ | |
| State machine safety | `bmc_check` ✓ | |
| Full functional proof | | Coq / Lean |
| Liveness properties | | TLA+ |
| Distributed consensus proof | | TLA+ / Ivy |
| Smart contract verification | | Dafny / Move Prover |
