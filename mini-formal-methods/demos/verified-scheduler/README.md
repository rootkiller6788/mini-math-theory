# Verified Scheduler

Demonstrates formal methods for a simple task scheduler:

1. **State machine model**: tasks have states (READY, RUNNING, BLOCKED, DONE)
2. **BMC check**: verify "no two tasks are RUNNING simultaneously" for bound k
3. **Loop invariants**: ready queue size never exceeds total tasks

```c
StateMachine sm;
sm_init(&sm, 4, 0);
// Model transitions between scheduler states
sm_add_transition(&sm, READY, RUNNING);
sm_add_transition(&sm, RUNNING, DONE);
// BMC: verify mutual exclusion property
bmc_check(&sm, no_two_running, 16, &result);
```

This mirrors how TLA+ models concurrent systems — but in C.
