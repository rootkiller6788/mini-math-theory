# Course Alignment: Formal Methods

## Primary Reference

**Software Foundations, Volume 1: Logical Foundations**
(UPenn / Benjamin Pierce et al.)

This project implements the runtime-checkable subset of formal methods in C:

| SF Concept | Our C Implementation |
|------------|---------------------|
| Preconditions / Postconditions | `contracts.h` — `REQUIRE`/`ENSURE` macros |
| Invariants | `invariants.h` — `LoopInvariant` runtime checker |
| Functional Specifications | `spec.h` — `Spec` with pre/post/inv |
| Proof by Exhaustion | `verification.h` — symbolic executor + BMC |
| Program Verification | `examples/` — verified stack, queue, sort |

## Also Aligned With

- **CMU 17-614/17-714 Formal Methods**: System modeling, abstraction, tool analysis
- **TLA+**: Specifying state machines (BMC module)
- **Alloy**: Relational modeling (contract framework)
- **Dafny**: Verified programs (our examples mirror Dafny verified code in C)
- **MIT 6.045J**: Complexity bridge (verification = exhaustive search = NP)

## Key Concepts Covered

| Concept | Module | Description |
|---------|--------|-------------|
| Hoare Logic | `contracts.h` | {P} C {Q} — pre/post conditions |
| Loop Invariants | `invariants.h` | Invariant establishment & preservation |
| Symbolic Execution | `verification.h` | Explore all paths for small input domains |
| Bounded Model Checking | `verification.h` | Check properties up to bound k |
| Specification | `spec.h` | Formal spec = pre + post + invariant |
