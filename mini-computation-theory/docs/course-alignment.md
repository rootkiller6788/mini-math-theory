# Course Alignment: MIT 6.045J Automata, Computability, and Complexity

## Module ⬄ Course Mapping

| This Module | MIT 6.045J Section | Topics |
|-------------|-------------------|--------|
| `dfa`       | Unit 1: Automata Theory | DFA creation, simulation, minimization, complement, union, intersection |
| `nfa`       | Unit 1: Nondeterminism | NFA with epsilon transitions, NFA→DFA subset construction |
| `regex`     | Unit 1: Regular Expressions | Thompson NFA construction, regex compile → match |
| `grammar`   | Unit 2: Context-Free Languages | CFG representation, CNF conversion, CYK algorithm |
| `pda`       | Unit 2: Pushdown Automata | PDA simulation, acceptance by final state |
| `turing_machine` | Unit 3: Computability Theory | TM simulator, undecidable problems, recognizers |
| `sat`       | Unit 4: Complexity Theory | DPLL algorithm, NP-completeness, SAT solving |

## Key Theorems Covered

| Theorem | Implementation |
|---------|---------------|
| Kleene's Theorem (regex = FA) | `regex.c`: Thompson construction |
| Subset Construction (NFA → DFA) | `nfa.c`: nfa_to_dfa() |
| CYK Algorithm (CFG membership) | `grammar.c`: grammar_cyk() |
| CNF Conversion | `grammar.c`: grammar_to_cnf() |
| Church-Turing Thesis | `turing_machine.c`: TM simulator |
| Cook-Levin Theorem (SAT is NP-C) | `sat.c`: DPLL solver |

## Also Aligned With

- Sipser: Introduction to the Theory of Computation
- CS172 (Berkeley): Computability and Complexity
