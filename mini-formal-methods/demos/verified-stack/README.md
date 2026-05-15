# Verified Stack

Design-by-contract stack implementation with:
- `REQUIRE`: stack not full before push, not empty before pop
- `ENSURE`: pushed value is on top, pop reduces size by 1
- `INVARIANT`: 0 <= top <= capacity always holds

Usage:
```c
#define CONTRACT_CHECK
#include "contracts.h"

Stack s;
stack_init(&s, 16);
stack_push(&s, 42);
int v = stack_pop(&s);
```

Contract violations abort with file/line/condition diagnostics.
Compile without `-DCONTRACT_CHECK` to strip all contract code for release.
