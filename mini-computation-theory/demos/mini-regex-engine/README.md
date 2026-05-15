# Mini Regex Engine

Uses the Thompson NFA construction algorithm to compile regular expressions into NFAs, then converts to DFA for efficient matching.

## Supported Syntax

- `a` — literal character
- `ab` — concatenation
- `a|b` — alternation
- `a*` — Kleene star (zero or more)
- `a+` — one or more
- `a?` — zero or one
- `.` — any single character

## Usage

```c
Regex* r = regex_compile("(a|b)*abb");
bool matched = regex_match(r, "aabb");  // true
bool no_match = regex_match(r, "aba");  // false
regex_free(r);
```

## Algorithm

1. Parse regex into AST
2. Build NFA via Thompson construction
3. Convert NFA → DFA (subset construction)
4. Simulate DFA on input string
