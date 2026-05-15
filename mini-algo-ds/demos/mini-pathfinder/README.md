# mini-pathfinder

A pathfinding demo implementing Dijkstra, BFS, and an introduction to the A* algorithm — inspired by network routing protocols and game AI.

## Overview

Finding the shortest path in a graph is one of the most practically important algorithmic problems. This demo implements the three classic approaches and discusses their real-world applications in networking (OSPF routing) and game development (A* pathfinding).

## Pathfinding Algorithms

### 1. BFS — Unweighted Shortest Path

**Algorithmic paradigm:** Graph traversal (queue-based)

Breadth-First Search finds the shortest path in terms of **number of edges** (hops) for unweighted graphs.

**Algorithm:**
```
bfs(G, s):
    dist[s] = 0
    queue.push(s)
    while queue not empty:
        u = queue.pop()
        for each neighbor v of u:
            if v not visited:
                dist[v] = dist[u] + 1
                prev[v] = u
                queue.push(v)
```

**Why it works:** BFS explores nodes in layers. All nodes at distance `d` are discovered before any node at distance `d+1`. When a node is first discovered, it is via the shortest path. See `docs/correctness-proofs.md` §7 for the full proof.

**Use case:** Social network "degrees of separation," web crawler URL frontier, unweighted grid tile movement.

### 2. Dijkstra — Weighted Shortest Path (OSPF Routing)

**Algorithmic paradigm:** Greedy + priority queue

Dijkstra's algorithm finds the shortest path in a graph with **non-negative edge weights**. Each edge has a cost (e.g., network latency, road length, fuel consumed).

**Algorithm:**
```
dijkstra(G, s):
    dist[s] = 0
    for all v != s: dist[v] = infinity
    pq = min_heap of (dist[v], v)
    while pq not empty:
        u = pq.extract_min()
        for each neighbor v of u:
            alt = dist[u] + w(u, v)
            if alt < dist[v]:
                dist[v] = alt
                prev[v] = u
                pq.decrease_key(v, alt)
```

**OSPF (Open Shortest Path First):** This is the real-world routing protocol used inside autonomous systems on the internet. Each router runs Dijkstra on a graph where vertices are routers and edge weights reflect link cost (bandwidth, latency, reliability). The result is a shortest-path tree used to populate the router's forwarding table.

**Use case:** GPS navigation (edge weight = travel time), network routing, resource allocation.

### 3. A* Search — Heuristic-Guided Shortest Path (Conceptual)

**Algorithmic paradigm:** Informed search (Dijkstra + heuristic)

A* improves on Dijkstra by using a **heuristic function** `h(v)` that estimates the remaining distance from node `v` to the goal. Instead of expanding nodes purely by `dist[v]`, A* uses:

```
f(v) = g(v) + h(v)
```

where:
- `g(v)` = actual distance from start to `v` (same as Dijkstra's `dist[v]`)
- `h(v)` = heuristic estimate from `v` to goal (e.g., Euclidean distance)
- `f(v)` = estimated total path cost through `v`

**Properties:**
- If `h(v)` is **admissible** (never overestimates the true remaining distance), A* is **optimal** — it finds the true shortest path.
- If `h(v)` is **consistent** (satisfies the triangle inequality: `h(u) <= w(u,v) + h(v)`), A* expands each node at most once.
- With `h(v) = 0` for all `v`, A* reduces to Dijkstra.
- With `h(v)` being a perfect oracle, A* expands only nodes on the optimal path.

**Use case:** Video game pathfinding (grid + Euclidean heuristic), robot motion planning, puzzle solving (15-puzzle with Manhattan distance).

## Demo Program

### Input Format

Reads a graph file describing a roadmap or network:
```
V 6                  # vertices 0..5
E 0 1 4              # edge 0-1 weight 4
E 0 2 2              # edge 0-2 weight 2
E 1 2 5
E 1 3 10
E 2 4 3
E 4 3 4
E 3 5 11
E 4 5 1
S 0 5                # find path from 0 to 5
```

### Output

```
=== BFS (unweighted, hop count) ===
Path: 0 -> 2 -> 4 -> 5
Hops: 3

=== Dijkstra (weighted) ===
Path: 0 -> 2 -> 4 -> 5
Cost: 0+2+3+1 = 6
Explored vertices: 6

=== A* (heuristic: Euclidean distance) ===
Heuristic: 0:h=10, 2:h=7, 4:h=3, 5:h=0
Path: 0 -> 2 -> 4 -> 5
Cost: 6 (optimal)
Explored vertices: 4 (vs Dijkstra's 6)
```

## Build and Run

```bash
make mini-pathfinder
./build/ex_mini-pathfinder [graph_file]
```

## Source Files

| File | Purpose |
|---|---|
| `demos/mini-pathfinder/main.c` | Demo entry point, file I/O |
| `demos/mini-pathfinder/pathfinder.c` | Pathfinding algorithm implementations |
| `demos/mini-pathfinder/pathfinder.h` | Pathfinder interface |
| `src/graph.c` | Graph representation (adjacency list) |
| `src/graph_algos.c` | BFS, Dijkstra implementations (shared) |
| `src/heap.c` | Min-heap for Dijkstra priority queue |

## Key Concepts

- **Relaxation:** The core operation in shortest-path algorithms. For edge `(u, v)` with weight `w`: if `dist[u] + w < dist[v]`, update `dist[v]`. This never invalidates upper-bound guarantees.
- **Negative edges:** BFS and Dijkstra assume non-negative edges. If negative edges exist but no negative cycles, use Bellman-Ford (O(VE)). If negative cycles exist, shortest "simple" paths are undefined (can loop indefinitely).
- **Heuristic design:** A* performance depends critically on the heuristic. A good heuristic reduces the search space dramatically. For grid maps, octile distance (allowing diagonal movement) is admissible and highly effective.
- **Routing protocol dynamics:** In OSPF, when a link fails, routers flood Link State Advertisements (LSAs), and every router recomputes Dijkstra from scratch. This converges in O(V^2 log V) per router.

## References

- CLRS Ch. 22 — Elementary Graph Algorithms (BFS, DFS)
- CLRS Ch. 24 — Single-Source Shortest Paths (Dijkstra, Bellman-Ford)
- Hart, Nilsson, Raphael, "A Formal Basis for the Heuristic Determination of Minimum Cost Paths" (1968) — A* paper
- RFC 2328 — OSPF Version 2
- MIT 6.006, Lecture 13 — Breadth-First Search
- MIT 6.006, Lectures 15–16 — Shortest Paths
