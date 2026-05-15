# Complexity Cheatsheet

Big-O complexity reference for all data structure operations and algorithms implemented in `mini-algo-ds`.

---

## Data Structure Operations

| Data Structure | Insert | Delete | Search | Find Min/Max | Space |
|---|---|---|---|---|---|
| Doubly Linked List | O(1) | O(1) | O(n) | O(n) | O(n) |
| Dynamic Array (Vector) | O(1) amortized | O(n) | O(1) by index, O(n) by value | O(n) | O(n) |
| Binary Search Tree (unbalanced) | O(h) / O(n) worst | O(h) / O(n) worst | O(h) / O(n) worst | O(h) / O(n) worst | O(n) |
| AVL Tree (balanced) | O(log n) | O(log n) | O(log n) | O(log n) | O(n) |
| Hash Table (chaining) | O(1) avg, O(n) worst | O(1) avg, O(n) worst | O(1) avg, O(n) worst | N/A | O(n) |
| Binary Heap (min-heap) | O(log n) | O(log n) extract-min | O(n) | O(1) find-min | O(n) |
| Union-Find (rank + compression) | O(alpha(n)) union | N/A | O(alpha(n)) find | N/A | O(n) |
| Graph (adjacency list) | O(1) add edge | O(degree) | O(degree) | N/A | O(V + E) |
| Trie | O(k) | O(k) | O(k) | N/A | O(alphabet_size * n * k) |

* h = tree height; k = key length; alpha(n) = inverse Ackermann (<= 5 for any practical n)

---

## Sorting Algorithms

| Algorithm | Best Case | Average Case | Worst Case | Space | Stable | In-Place |
|---|---|---|---|---|---|---|
| Insertion Sort | O(n) | O(n^2) | O(n^2) | O(1) | Yes | Yes |
| Merge Sort | O(n log n) | O(n log n) | O(n log n) | O(n) | Yes | No |
| Quick Sort | O(n log n) | O(n log n) | O(n^2) | O(log n) | No | Yes |
| Randomized Quick Sort | O(n log n) | O(n log n) | O(n log n) expected | O(log n) | No | Yes |
| Heap Sort | O(n log n) | O(n log n) | O(n log n) | O(1) | No | Yes |
| Counting Sort | O(n + k) | O(n + k) | O(n + k) | O(k) | Yes | No |
| Radix Sort | O(d(n + k)) | O(d(n + k)) | O(d(n + k)) | O(n + k) | Yes | No |
| Bucket Sort | O(n + k) | O(n + k) | O(n^2) | O(n + k) | Yes | No |

* k = range of keys; d = number of digits

---

## Searching Algorithms

| Algorithm | Time (Unsorted) | Time (Sorted) | Space |
|---|---|---|---|
| Linear Search | O(n) | N/A | O(1) |
| Binary Search | N/A | O(log n) | O(1) |
| Jump Search | N/A | O(sqrt(n)) | O(1) |
| Quickselect (k-th order statistic) | O(n) expected, O(n^2) worst | N/A | O(log n) |

---

## Graph Algorithms

### Traversal

| Algorithm | Time | Space | Use Case |
|---|---|---|---|
| BFS | O(V + E) | O(V) | Shortest path (unweighted), level-order |
| DFS | O(V + E) | O(V) | Cycle detection, topological sort, connected components |
| Topological Sort (DFS) | O(V + E) | O(V) | Dependency ordering |

### Shortest Path — Single Source

| Algorithm | Time | Space | Handles Negative Edges? | Notes |
|---|---|---|---|---|
| BFS (unweighted) | O(V + E) | O(V) | N/A | Edges must have uniform weight |
| Dijkstra | O((V + E) log V) | O(V) | No | Requires non-negative weights |
| Bellman-Ford | O(VE) | O(V) | Yes | Also detects negative cycles |
| DAG Shortest Path | O(V + E) | O(V) | Yes | Only for DAGs |

### Shortest Path — All Pairs

| Algorithm | Time | Space | Notes |
|---|---|---|---|
| Dijkstra (run V times) | O(V(V + E) log V) | O(V^2) | Non-negative edges only |
| Bellman-Ford (run V times) | O(V^2 E) | O(V^2) | Handles negatives |
| Floyd-Warshall | O(V^3) | O(V^2) | DP approach, dense graphs |
| Johnson | O(VE + V^2 log V) | O(V^2) | Reweighting + Dijkstra |

### Minimum Spanning Tree

| Algorithm | Time | Space | Notes |
|---|---|---|---|
| Kruskal | O(E log E) | O(V) | Uses Union-Find; good for sparse |
| Prim | O((V + E) log V) | O(V) | Uses priority queue; good for dense |

### Maximum Flow

| Algorithm | Time | Notes |
|---|---|---|
| Ford-Fulkerson | O(E * |f|) | |f| = max flow value |
| Edmonds-Karp | O(V E^2) | BFS-based |

---

## Dynamic Programming

| Algorithm | Time | Space | Dimension / State | Recurrence |
|---|---|---|---|---|
| Fibonacci (memoized) | O(n) | O(n) | `dp[i] = dp[i-1] + dp[i-2]` | 1D |
| 0/1 Knapsack | O(nW) | O(nW) / O(W) | `dp[i][w] = max(dp[i-1][w], v_i + dp[i-1][w - w_i])` | 2D, optimizable to 1D |
| Longest Common Subsequence | O(mn) | O(mn) / O(min(m,n)) | `dp[i][j] = dp[i-1][j-1] + 1` if match else `max(dp[i-1][j], dp[i][j-1])` | 2D |
| Edit Distance | O(mn) | O(mn) / O(min(m,n)) | `dp[i][j] = min(insert, delete, substitute)` | 2D |
| Rod Cutting | O(n^2) | O(n) | `dp[i] = max_{1 <= j <= i}(p_j + dp[i-j])` | 1D |
| Matrix Chain Multiplication | O(n^3) | O(n^2) | `dp[i][j] = min_{i <= k < j}(dp[i][k] + dp[k+1][j] + dims)` | 2D |
| Weighted Interval Scheduling | O(n log n) | O(n) | `dp[i] = max(w_i + dp[p(i)], dp[i-1])` | 1D + binary search |
| Longest Increasing Subsequence | O(n log n) | O(n) | Patience sorting / DP + binary search | 1D |
| Coin Change | O(nV) | O(V) | `dp[v] = min(dp[v], 1 + dp[v - c_i])` | 1D |
| Egg Dropping | O(nk) | O(nk) | `dp[n][k] = min_{x}(1 + max(dp[n-1][x-1], dp[n][k-x]))` | 2D |

* n = number of items; W = knapsack capacity; m,n = string lengths; V = target value

---

## Greedy Algorithms

| Algorithm | Time | Correctness Criterion | Proof Technique |
|---|---|---|---|
| Activity Selection | O(n log n) | Earliest finish time | Exchange argument |
| Huffman Coding | O(n log n) | Optimal prefix codes | Greedy choice + optimal substructure |
| Fractional Knapsack | O(n log n) | Value/weight ratio descending | Exchange argument |
| Coin Change (canonical coins) | O(n) | Greedy works for canonical coin systems | Counterexample if non-canonical |
| Interval Partitioning | O(n log n) | Earliest start time (minimize classrooms) | Lower bound on depth |

---

## Miscellaneous

| Algorithm | Time | Space | Notes |
|---|---|---|---|
| Sieve of Eratosthenes (primes) | O(n log log n) | O(n) | |
| Euclidean Algorithm (GCD) | O(log min(a,b)) | O(1) | |
| Fast Exponentiation | O(log n) | O(1) | |
| Karatsuba Multiplication | O(n^{log_2 3}) ~ O(n^{1.585}) | O(n) | Recursive |

---

## Amortized Analysis Summary

| Data Structure / Operation | Aggregate Cost | Amortized Cost per Op |
|---|---|---|
| Dynamic Array push | O(n) total for n pushes | O(1) |
| Binary Heap build (heapify) | O(n) | O(1) per element |
| Union-Find with path compression (m ops, n elements) | O(m · alpha(n)) | O(alpha(n)) |
| Fibonacci Heap decrease-key | O(n log n + m) total | Theta(1) |

---

## Space Complexity Quick Reference

| Data Structure | Space Formula | Example (n = 1 million) |
|---|---|---|
| Linked List | 2n pointers + n data | ~24 MB (nodes with 2 pointers + int) |
| Dynamic Array | n * sizeof(element) | 4 MB (int32) |
| BST | 2n pointers + n data | ~24 MB |
| Hash Table (chaining) | n buckets + n entries + pointers | ~32 MB at load factor 0.75 |
| Adjacency List (graph) | V list heads + 2E edges | 2E edges for undirected, E for directed |
| Heap (array) | n * sizeof(element) | 4 MB (int32) |
| Trie (alphabet size 26, avg key len 5) | ~130n nodes worst case | Can be large for many long strings |
