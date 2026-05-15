# Software Foundations Mapping

## Volume 1: Logical Foundations (SF)

The UPenn Software Foundations series uses Coq to teach:
- Functional programming in Gallina
- Logic and inductive propositions
- Program verification with Hoare logic

Our C implementation mirrors SF concepts in a runtime-checkable form:

| SF Chapter | C Counterpart |
|------------|---------------|
| Basics (nat, bool, lists) | Built-in C types + our data structures |
| Induction | `proofs.md` in discrete-math, induction demos |
| Lists | Verified stack/queue with invariants |
| Poly (polymorphism) | `void*` generic pointers in `contracts.h` |
| Logic (Prop) | `contracts.h` — REQUIRE/ENSURE/INVARIANT |
| Imp (simple imperative language) | `verification.h` — symbolic execution |
| Hoare (Hoare logic) | `spec.h` — pre/post/invariant checks |
| Hoare2 (decorated programs) | `examples/` — decorated with contracts |

## Key Difference: Coq vs Runtime C

Coq proves properties statically at compile time.
Our C framework checks them dynamically at runtime.

The C approach trades compile-time guarantees for:
- Immediate executability
- Integration with production C code
- Educational clarity (see the contract fail, not just a proof error)
