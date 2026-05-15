# mini-algo-ds

MIT 6.006 + 6.046J based algorithms &amp; data structures in C

A from-scratch implementation of fundamental algorithms and data structures as taught in MIT&#39;s Introduction to Algorithms (6.006) and Design and Analysis of Algorithms (6.046J). All data structures and algorithms are implemented in standard C with no external dependencies.

## Data Structures

| Data Structure | Header | Key Operations | CS Applications |
|---|---|---|---|
| Doubly Linked List | `linked_list.h` | insert_at, remove_at, find | LRU cache, free lists, undo stacks |
| Dynamic Array | `vector.h` | push, pop, at | adjacency list, heap backing store |
| Binary Search Tree | `bst.h` | insert, search, delete | ordered map, range queries, database index |
| AVL Tree | `avl.h` | insert, search, delete (self-balancing) | ordered map with O(log n) worst-case |
| Hash Table | `hash_table.h` | put, get, remove (chaining) | symbol table, dedup, LRU cache lookup |
| Binary Heap | `heap.h` | push, pop, heapify | priority queue, scheduler, Dijkstra |
| Union-Find | `union_find.h` | find, union (rank + path compression) | Kruskal MST, connected components |
| Graph (Adjacency List) | `graph.h` | add_edge, neighbors | network routing, social graph |
| Trie | `trie.h` | insert, search, prefix | autocomplete, IP routing, spell check |
| B-Tree (conceptual) | `btree.h` | insert, search, range | database indexes, filesystems |

## Algorithms

### Sorting

| Algorithm | File | Time (Best) | Time (Avg) | Time (Worst) | Space | Stable |
|---|---|---|---|---|---|---|
| Insertion Sort | `sort.c` | O(n) | O(n^2) | O(n^2) | O(1) | Yes |
| Merge Sort | `sort.c` | O(n log n) | O(n log n) | O(n log n) | O(n) | Yes |
| Quick Sort | `sort.c` | O(n log n) | O(n log n) | O(n^2) | O(log n) | No |
| Heap Sort | `sort.c` | O(n log n) | O(n log n) | O(n log n) | O(1) | No |
| Counting Sort | `sort.c` | O(n + k) | O(n + k) | O(n + k) | O(k) | Yes |
| Radix Sort | `sort.c` | O(d(n + k)) | O(d(n + k)) | O(d(n + k)) | O(n + k) | Yes |

### Searching

| Algorithm | File | Time | Space |
|---|---|---|---|
| Binary Search | `search.c` | O(log n) | O(1) |
| Linear Search | `search.c` | O(n) | O(1) |
| Quickselect (k-th smallest) | `search.c` | O(n) avg | O(log n) |

### Graph Algorithms

| Algorithm | File | Time | Space | Notes |
|---|---|---|---|---|
| BFS | `graph_algos.c` | O(V + E) | O(V) | Unweighted shortest path |
| DFS | `graph_algos.c` | O(V + E) | O(V) | Topological sort, cycle detection |
| Dijkstra | `graph_algos.c` | O((V+E) log V) | O(V) | Non-negative weights |
| Bellman-Ford | `graph_algos.c` | O(VE) | O(V) | Handles negative weights |
| Kruskal MST | `graph_algos.c` | O(E log E) | O(V) | Uses Union-Find |
| Prim MST | `graph_algos.c` | O((V+E) log V) | O(V) | Uses priority queue |
| Floyd-Warshall | `graph_algos.c` | O(V^3) | O(V^2) | All-pairs shortest path |

### Dynamic Programming

| Algorithm | File | Time | Space | Notes |
|---|---|---|---|---|
| 0/1 Knapsack | `dp.c` | O(nW) | O(nW) or O(W) | Pseudo-polynomial |
| LCS | `dp.c` | O(mn) | O(mn) | Longest Common Subsequence |
| Edit Distance | `dp.c` | O(mn) | O(mn) | Levenshtein |
| Rod Cutting | `dp.c` | O(n^2) | O(n) | |
| Matrix Chain | `dp.c` | O(n^3) | O(n^2) | Optimal parenthesization |
| Weighted Interval Scheduling | `dp.c` | O(n log n) | O(n) | Sort + DP |

### Greedy Algorithms

| Algorithm | File | Time | Notes |
|---|---|---|---|
| Activity Selection | `greedy.c` | O(n log n) | Earliest finish time |
| Huffman Coding | `greedy.c` | O(n log n) | Optimal prefix codes |
| Fractional Knapsack | `greedy.c` | O(n log n) | Value/weight ratio |

## How to Build and Run

### Prerequisites

- GCC or Clang (C99 or later)
- GNU Make

### Build All Demos

```
make all
```

### Build and Run a Specific Demo

```
make mini-scheduler
./build/mini-scheduler
```

### Run All Tests

```
make test
```

### Clean Build Artifacts

```
make clean
```

### Compiler Flags

All code compiles with `-Wall -Wextra -std=c99 -O2`. Debug builds use `-g -O0`.

## Directory Structure

```
mini-algo-ds/
├── README.md                   # This file
├── Makefile                    # Build system
├── include/                    # Header files
│   ├── linked_list.h
│   ├── vector.h
│   ├── bst.h
│   ├── avl.h
│   ├── heap.h
│   ├── hash_table.h
│   ├── union_find.h
│   ├── graph.h
│   ├── trie.h
│   ├── btree.h
│   ├── sort.h
│   ├── search.h
│   ├── graph_algos.h
│   ├── dp.h
│   └── greedy.h
├── src/                        # Implementation files
│   ├── linked_list.c
│   ├── vector.c
│   ├── bst.c
│   ├── avl.c
│   ├── heap.c
│   ├── hash_table.c
│   ├── union_find.c
│   ├── graph.c
│   ├── trie.c
│   ├── btree.c
│   ├── sort.c
│   ├── search.c
│   ├── graph_algos.c
│   ├── dp.c
│   └── greedy.c
├── tests/                      # Unit tests
│   └── test_all.c
├── examples/                   # Standalone example programs
│   ├── sort_demo.c
│   ├── graph_demo.c
│   ├── dp_demo.c
│   └── greedy_demo.c
├── benches/                    # Micro-benchmarks
│   └── bench_all.c
├── demos/                      # Application demos
│   ├── mini-scheduler/         # Task scheduler
│   ├── mini-cache/             # LRU cache
│   ├── mini-index/             # Database index
│   ├── mini-pathfinder/        # Pathfinding
│   └── mini-autocomplete/      # Autocomplete
└── docs/                       # Documentation
    ├── course-alignment.md
    ├── complexity-cheatsheet.md
    └── correctness-proofs.md
```

## References

- Cormen, Leiserson, Rivest, Stein &mdash; *Introduction to Algorithms* (CLRS), 3rd/4th Edition
- MIT 6.006 &mdash; Introduction to Algorithms (Fall 2011), Prof. Erik Demaine, Prof. Srini Devadas
- MIT 6.046J &mdash; Design and Analysis of Algorithms (Spring 2015), Prof. Erik Demaine, Prof. Srini Devadas
