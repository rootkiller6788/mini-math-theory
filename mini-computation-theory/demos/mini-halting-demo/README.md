# Mini Halting Demo

Demonstrates the limits of computation using a Turing machine simulator.

## Recognizers

### a^n b^n Recognizer
Shows a non-regular language that IS decidable by a Turing machine.
- Input: strings of a's followed by b's
- Accepts: `{aⁿbⁿ | n ≥ 1}`
- Not recognizable by any DFA/PDA (requires counting)

### w#w^R Recognizer  
Shows a non-context-free language that IS Turing-decidable.
- Input: w#w^R where w ∈ {a,b}* and w^R is the reverse
- This language is NOT context-free but IS decidable

### Binary Increment
Shows a computable function: f(x) = x + 1 in binary.
Demonstrates that Turing machines can compute functions, not just recognize languages.

## The Halting Problem

The halting problem is undecidable: no Turing machine can determine
whether an arbitrary TM halts on an arbitrary input.

Our simulator addresses this with a **step limit** parameter to avoid infinite loops.
