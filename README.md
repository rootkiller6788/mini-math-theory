# Mini Math Theory

A collection of **from-scratch, zero-dependency C implementations** of university-level mathematical theory for computer science. Each module maps to MIT (and other top-tier university) courses, bridging theory and practice by translating textbook equations into runnable C code.

## Modules

| Module | Topics | Key Courses |
|--------|--------|-------------|
| [mini-algo-ds](mini-algo-ds/) | Data structures, sorting, graph algorithms, dynamic programming, greedy | MIT 6.006, MIT 6.046J |
| [mini-computation-theory](mini-computation-theory/) | DFA/NFA, regex engine, CFG/PDA, Turing machines, SAT solver (DPLL) | MIT 6.045J, Sipser |
| [mini-discrete-math](mini-discrete-math/) | Logic, number theory, sets/relations, graph theory, combinatorics, Boolean algebra | MIT 6.042J |
| [mini-formal-methods](mini-formal-methods/) | Hoare logic contracts, invariants, symbolic execution, bounded model checking | UPenn Software Foundations, CMU |
| [mini-information-theory](mini-information-theory/) | Entropy, mutual information, coding theory, compression, error correction | MIT 6.441, Cover & Thomas |
| [mini-linear-algebra](mini-linear-algebra/) | Vectors, matrices, decompositions (LU/QR/Cholesky), eigenvalues, SVD, sparse operations | MIT 18.06, MIT 18.065, Stanford CS229 |
| [mini-prob-stats](mini-prob-stats/) | Probability, distributions, Bayesian inference, hypothesis testing, regression, Monte Carlo | MIT 18.05, Stanford CS229 |
| [mini-system-theory](mini-system-theory/) | State-space models, stability, PID control, system dynamics, queuing theory, tradespace analysis | MIT 16.842, MIT 6.241J |

## Design Philosophy

- **Zero external dependencies** — pure C (C99/C11), only `libc` and `libm`
- **Self-contained modules** — each directory has its own `Makefile`, `include/`, `src/`, `examples/`, `demos/`, `tests/`
- **Theory-to-code mapping** — every module includes `docs/` with course-alignment notes
- **Practical demos** — schedulers, pathfinders, compressors, recommenders, controllers, and more

## Building

Each module is standalone. Navigate to a module directory and run:

```bash
cd mini-algo-ds
make all    # build everything
make test   # run tests
```

Requires **GCC** and **GNU Make**.

## Project Structure

```
mini-math-theory/
├── mini-algo-ds/              # Algorithms & Data Structures
├── mini-computation-theory/   # Automata, Computability, Complexity
├── mini-discrete-math/        # Discrete Mathematics
├── mini-formal-methods/       # Formal Methods & Verification
├── mini-information-theory/   # Information Theory
├── mini-linear-algebra/       # Linear Algebra & Matrix Methods
├── mini-prob-stats/           # Probability & Statistics
└── mini-system-theory/        # Systems Engineering & Control Theory
```

## License

MIT
