# Course Alignment

This document maps each module of `mini-algo-ds` to the corresponding units in MIT 6.006 (Introduction to Algorithms) and MIT 6.046J (Design and Analysis of Algorithms), as well as relevant chapters in CLRS (*Introduction to Algorithms*, 3rd/4th Edition).

---

## MIT 6.006 — Introduction to Algorithms

| 6.006 Unit | Topics | mini-algo-ds Module | Files |
|---|---|---|---|
| **Unit 1: Algorithmic Thinking** | Peak finding, asymptotic complexity, recurrences, Master Theorem | `search.c` — binary search, linear search, quickselect; all complexity analysis in docstrings | `src/search.c`, `docs/complexity-cheatsheet.md` |
| **Unit 2: Sorting & Trees** | Insertion sort, merge sort, heaps, heapsort, BST, AVL trees, counting sort, radix sort | `sort.c` — all sorting algorithms; `heap.c` — min/max heap; `bst.c` — BST with traversal; `avl.c` — self-balancing BST | `src/sort.c`, `src/heap.c`, `src/bst.c`, `src/avl.c` |
| **Unit 3: Hashing** | Hash functions, chaining, open addressing, universal hashing, perfect hashing | `hash_table.c` — chaining-based hash table with configurable hash functions | `src/hash_table.c` |
| **Unit 4: Graph Algorithms** | BFS, DFS, topological sort, Dijkstra, Bellman-Ford | `graph_algos.c` — BFS, DFS, Dijkstra, Bellman-Ford, topological sort; `graph.c` — adjacency list representation | `src/graph_algos.c`, `src/graph.c` |
| **Unit 5: Dynamic Programming** | Memoization, Fibonacci, text justification, knapsack, LCS | `dp.c` — 0/1 knapsack, LCS, edit distance, rod cutting, matrix chain, weighted interval scheduling | `src/dp.c` |
| **Unit 6: Greedy Algorithms** | Activity selection, Huffman coding, fractional knapsack | `greedy.c` — activity selection, Huffman coding, fractional knapsack; `union_find.c` — union-by-rank with path compression | `src/greedy.c`, `src/union_find.c` |

---

## CLRS Chapter Mapping

| CLRS Chapter | Topic | mini-algo-ds Module |
|---|---|---|
| Ch. 2 — Getting Started | Insertion sort, algorithm analysis | `src/sort.c` (insertion sort) |
| Ch. 4 — Divide and Conquer | Recurrences, max-subarray, Strassen | `src/sort.c` (merge sort, quick sort recursions) |
| Ch. 6 — Heapsort | Heaps, priority queues | `src/heap.c` |
| Ch. 7 — Quicksort | Quicksort, randomized quicksort | `src/sort.c` (quicksort) |
| Ch. 8 — Sorting in Linear Time | Counting sort, radix sort, bucket sort | `src/sort.c` (counting sort, radix sort) |
| Ch. 9 — Medians and Order Statistics | Selection in expected linear time | `src/search.c` (quickselect) |
| Ch. 10 — Elementary Data Structures | Stacks, queues, linked lists, trees | `src/linked_list.c`, `src/vector.c` |
| Ch. 11 — Hash Tables | Chaining, open addressing, hashing | `src/hash_table.c` |
| Ch. 12 — Binary Search Trees | BST operations, traversal | `src/bst.c` |
| Ch. 13 — Red-Black Trees | Balanced BSTs | `src/avl.c` (AVL as representative balanced BST) |
| Ch. 15 — Dynamic Programming | Rod cutting, matrix-chain, LCS, optimal BST | `src/dp.c` |
| Ch. 16 — Greedy Algorithms | Activity selection, Huffman codes, matroids | `src/greedy.c` |
| Ch. 18 — B-Trees | B-Tree operations, disk model | `src/btree.c` |
| Ch. 19 — Fibonacci Heaps | Amortized analysis, decrease-key | `src/heap.c` (binary heap; Fibonacci heap noted) |
| Ch. 21 — Disjoint Sets | Union-Find data structure | `src/union_find.c` |
| Ch. 22 — Elementary Graph Algorithms | BFS, DFS | `src/graph_algos.c` |
| Ch. 23 — Minimum Spanning Trees | Kruskal, Prim | `src/graph_algos.c` |
| Ch. 24 — Single-Source Shortest Paths | Dijkstra, Bellman-Ford | `src/graph_algos.c` |
| Ch. 25 — All-Pairs Shortest Paths | Floyd-Warshall, Johnson | `src/graph_algos.c` |
| Ch. 34 — NP-Completeness | P vs NP, reductions | (theory discussion in `docs/correctness-proofs.md`) |

---

## MIT 6.046J — Design and Analysis of Algorithms (Additional Topics)

| 6.046J Topic | Relevance | mini-algo-ds Module |
|---|---|---|
| Amortized Analysis | Potential method, aggregate analysis | `src/union_find.c` (inverse Ackermann bounds); `docs/correctness-proofs.md` |
| Skip Lists | Probabilistic alternative to balanced trees | Conceptual reference in `include/`; linked list with express lanes |
| Network Flow | Ford-Fulkerson, Edmonds-Karp, max-flow min-cut | Planned extension to `src/graph_algos.c` |
| Linear Programming | Simplex, duality | Conceptual notes in `docs/` |
| NP-Completeness | Reductions, Cook-Levin | Theory in `docs/correctness-proofs.md` |
| Approximation Algorithms | Vertex cover, set cover, TSP | Planned extension to `src/greedy.c` |
| Randomized Algorithms | Quicksort analysis, skip lists, universal hashing | `src/sort.c` (randomized quicksort), `src/hash_table.c` (universal hash) |
| Van Emde Boas Trees | O(log log U) priority queue | Advanced reference only |
| Competitive Analysis | Self-organizing lists, paging | `demos/mini-cache/` (LRU vs optimal) |

---

## Learning Path

For self-study, follow this order:

1. **`search.c` + `sort.c`** — Build intuition for complexity and divide-and-conquer.
2. **`linked_list.c` + `vector.c`** — Elementary data structures.
3. **`bst.c` + `avl.c`** — Tree-based ordered maps.
4. **`heap.c`** — Priority queues and heapsort.
5. **`hash_table.c`** — Hashing for O(1) lookups.
6. **`graph.c` + `graph_algos.c`** — Graph representation and traversal.
7. **`union_find.c`** — Disjoint-set for Kruskal.
8. **`dp.c`** — Dynamic programming patterns.
9. **`greedy.c`** — Greedy algorithm design.
10. **`trie.c` + `btree.c`** — Advanced tree structures.

Each module is self-contained and includes extensive comments referencing relevant CLRS sections and MIT lecture numbers.
