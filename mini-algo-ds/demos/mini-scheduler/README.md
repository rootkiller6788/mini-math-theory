# mini-scheduler

A task scheduling demo that combines priority queues, greedy algorithms, and dynamic programming.

## Overview

This demo implements three classic scheduling strategies, each drawn from a different algorithmic paradigm. Together they model the core decision problems in operating system and job scheduling.

## Scheduling Strategies

### 1. Priority Scheduling (Min-Heap)

**Algorithmic paradigm:** Greedy via priority queue

Uses a **min-heap** (`heap.h`) to always execute the task with the earliest deadline or highest priority first. Each task has a `(priority, deadline, duration)` tuple. The heap maintains the invariant that the root is the task with the smallest key (deadline or priority value).

**Operations:**
- `push`: O(log n) — insert a new task
- `pop`: O(log n) — extract and execute the highest-priority task
- `peek`: O(1) — inspect without removing

**Use case:** Real-time task scheduling in operating systems (e.g., Earliest Deadline First — EDF).

### 2. Activity Selection (Greedy)

**Algorithmic paradigm:** Greedy (earliest finish time)

Given a set of `n` non-overlapping tasks, each with a start and finish time, select the maximum number of tasks that can be scheduled without conflicts.

**Algorithm:**
1. Sort tasks by finish time (O(n log n))
2. Iterate: select the task with the earliest finish time that starts after the last selected task finishes
3. Greedy choice property guarantees optimality (see `docs/correctness-proofs.md` §3)

**Use case:** Conference room scheduling, course timetabling.

### 3. Weighted Interval Scheduling (DP)

**Algorithmic paradigm:** Dynamic Programming

Each task has a **weight** (profit, priority, or importance) in addition to start/finish times. The goal is to select a subset of mutually compatible tasks that maximizes total weight.

**DP Formulation:**
```
dp[i] = max(weight[i] + dp[p(i)], dp[i-1])
```
where `p(i)` is the largest index `j < i` such that task `j` does not overlap with task `i` (found via binary search).

- **Sort** by finish time: O(n log n)
- **Compute `p(i)`** via binary search: O(n log n)
- **Fill dp table:** O(n)
- **Total:** O(n log n)

**Use case:** Ad placement scheduling (maximize revenue), job scheduling with variable payoffs.

## Demo Program

### Input Format

The demo reads a CSV file of tasks:
```
name, start, finish, weight, priority, duration
task_a, 0, 3, 100, 1, 3
task_b, 2, 5, 50, 3, 3
task_c, 4, 7, 200, 2, 3
```

### Output

```
=== Priority Schedule (by deadline) ===
Executing: task_a (priority=1, duration=3)
Executing: task_c (priority=2, duration=3)
Executing: task_b (priority=3, duration=3)

=== Greedy Activity Selection ===
Selected 2 non-overlapping tasks:
  task_a [0, 3]
  task_c [4, 7]
Maximum tasks scheduled: 2

=== Weighted Interval Scheduling ===
Selected tasks: task_c [4,7] weight=200, task_a [0,3] weight=100
Maximum total weight: 300
```

## Build and Run

```bash
make mini-scheduler
./build/ex_mini-scheduler
```

## Source Files

| File | Purpose |
|---|---|
| `demos/mini-scheduler/main.c` | Demo entry point and I/O |
| `demos/mini-scheduler/scheduler.c` | Core scheduling logic |
| `demos/mini-scheduler/scheduler.h` | Scheduler interface |
| `src/heap.c` | Min-heap implementation (shared) |
| `src/greedy.c` | Activity selection (shared) |
| `src/dp.c` | Weighted interval scheduling (shared) |

## Key Concepts

- **Priority queue as scheduler backbone:** The OS maintains a min-heap of ready tasks. The scheduler extracts the top task at each time slice.
- **Greedy vs DP tradeoff:** When tasks have equal weight, greedy is optimal and simpler. When tasks have varying importance, DP is required.
- **Earliest Deadline First (EDF):** Optimal for preemptive uniprocessor scheduling when all tasks are known.

## References

- CLRS Ch. 16 — Greedy Algorithms (Activity Selection)
- CLRS Ch. 15 — Dynamic Programming (Weighted Interval Scheduling)
- MIT 6.006, Lecture 14 — Interval Scheduling
- MIT 6.046J, Lecture 1 — Weighted Interval Scheduling
