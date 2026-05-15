# Mini Parser

Demonstrates context-free grammar parsing using the CYK (Cocke-Younger-Kasami) algorithm.

## Usage

```c
Grammar* g = grammar_create('S');
grammar_add_production(g, 'S', 'A', 'B');
grammar_add_production(g, 'A', 'a');
grammar_add_production(g, 'B', 'b');
grammar_to_cnf(g);
Grammar* cnf = grammar_to_cnf(g);
bool accepted = grammar_cyk(cnf, "ab");  // true
```

## Algorithm

1. Convert grammar to CNF
2. Build CYK table (dynamic programming)
3. Check if start symbol generates the entire string

Time complexity: O(n³ · |G|) where n = input length, |G| = grammar size
