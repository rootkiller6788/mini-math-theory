# Correctness Proofs

Proof sketches for the correctness of key algorithms implemented in `mini-algo-ds`. Each sketch follows the structure: **invariant / property → proof by induction / exchange argument / cut property → conclusion**.

---

## 1. Quicksort Correctness — Partition Invariant

### Algorithm Recap

```
quicksort(A, lo, hi):
    if lo < hi:
        p = partition(A, lo, hi)
        quicksort(A, lo, p - 1)
        quicksort(A, p + 1, hi)

partition(A, lo, hi):
    pivot = A[hi]
    i = lo - 1
    for j = lo to hi - 1:
        if A[j] <= pivot:
            i++
            swap(A[i], A[j])
    swap(A[i+1], A[hi])
    return i + 1
```

### Partition Invariant

At the start of each iteration of the `for` loop (index `j`), the array `A[lo..hi-1]` is divided into three regions:

1. **`A[lo..i]`** — elements <= pivot (the "left" partition)
2. **`A[i+1..j-1]`** — elements > pivot (the "right" partition)
3. **`A[j..hi-1]`** — unprocessed elements

And `A[hi]` holds the pivot value.

Formally:

```
∀ k ∈ [lo, i] : A[k] <= pivot
∀ k ∈ [i+1, j-1] : A[k] > pivot
```

### Base Case

When `j = lo`, we have `i = lo - 1`. Both regions 1 and 2 are empty, so the invariant holds vacuously.

### Inductive Step

Assume the invariant holds at the start of iteration `j`. Consider element `A[j]`:

- **Case 1:** `A[j] <= pivot`
  - Increment `i` (now `A[i]` is the first element of region 2, which is > pivot)
  - Swap `A[i]` and `A[j]`. This places the new <= element at the end of region 1 and the > element at position `j` (or extends region 2 to `j`)
  - Region 1 grows by one; region 2 shifts right; invariant is restored.

- **Case 2:** `A[j] > pivot`
  - Do nothing; `j` increments. Region 2 grows by one upon the next iteration. Invariant holds.

### Termination

When `j = hi`, regions 1 and 2 cover `A[lo..hi-1]`. The final swap places the pivot between them:

```
∀ k ∈ [lo, p-1] : A[k] <= pivot
A[p] = pivot
∀ k ∈ [p+1, hi] : A[k] > pivot
```

### Correctness

By structural induction on the recursion tree, `quicksort` correctly sorts the array. Each recursive call operates on a subarray of size strictly less than the parent, guaranteeing termination. The partition ensures that after each call, the pivot is in its final sorted position.

### Worst-Case and Randomization

The worst case O(n^2) occurs when the pivot is always the minimum or maximum (e.g., already sorted array with last-element pivot). Randomized quicksort selects the pivot uniformly at random, giving **expected** O(n log n) complexity. By linearity of expectation, the expected number of comparisons is at most `2n ln n`.

---

## 2. Dijkstra's Algorithm Correctness — Relaxation + Greedy Choice

### Algorithm

```
dijkstra(G, s):
    dist[s] = 0
    for all v != s: dist[v] = infinity
    S = empty set
    Q = priority queue of all vertices keyed by dist
    while Q not empty:
        u = extract_min(Q)
        add u to S
        for each neighbor v of u:
            if dist[u] + w(u,v) < dist[v]:
                dist[v] = dist[u] + w(u,v)    // relax
                decrease_key(Q, v, dist[v])
```

### Invariant (Relaxation)

At all times, `dist[v]` is either infinity or the length of some path from `s` to `v`. Formally:

```
dist[v] >= delta(s, v)  for all v
```

where `delta(s, v)` is the true shortest-path distance from `s` to `v`.

**Proof by induction on the number of relaxation steps:** Initially, `dist[s] = 0` and `dist[v] = infinity` for `v != s`, so `dist[s] = delta(s, s)` and `dist[v] >= delta(s, v)`. Each relaxation `dist[v] = min(dist[v], dist[u] + w(u,v))` preserves the invariant because `dist[u] + w(u,v)` is the length of an actual path from `s` to `v` via `u`.

### Greedy Choice Property

**Claim:** When a vertex `u` is extracted from the priority queue (added to the set `S`), `dist[u] = delta(s, u)`, and this value never changes thereafter.

**Proof by contradiction:** Suppose `u` is the first vertex extracted for which `dist[u] > delta(s, u)`. Let `P` be a true shortest path from `s` to `u`. Let `(x, y)` be the first edge on `P` such that `x` is in `S` and `y` is not in `S`. At the moment `u` is extracted:

- `dist[x] = delta(s, x)` (since `x` was extracted before `u`, and `u` is the first "bad" vertex)
- Edge `(x, y)` was relaxed when `x` was added to `S`, so `dist[y] <= dist[x] + w(x,y) = delta(s, x) + w(x,y) = delta(s, y)`
- Since edge weights are non-negative and `y` precedes `u` on the shortest path: `delta(s, y) <= delta(s, u)`
- Therefore `dist[y] <= delta(s, y) <= delta(s, u) < dist[u]`
- But `u` was extracted first, meaning `dist[u] <= dist[y]`. Contradiction.

Thus no such `u` exists. Every extracted vertex has `dist[u] = delta(s, u)`.

### Non-Negative Weights

The proof critically depends on **non-negative edge weights**. If `w(x,y) < 0`, then `delta(s, y) <= delta(s, u)` is not guaranteed, and the greedy extraction fails. This is why Dijkstra does not work on graphs with negative edges.

---

## 3. Kruskal's Algorithm Correctness — Cut Property

### Algorithm

```
kruskal(G):
    T = empty set
    sort edges by weight ascending
    for each edge (u, v) in sorted order:
        if find(u) != find(v):
            add (u, v) to T
            union(u, v)
    return T
```

### Cut Property

**Theorem:** Let `S` be any subset of vertices of a connected graph `G`, and let `e` be the minimum-weight edge crossing the cut `(S, V \ S)`. Then `e` belongs to some minimum spanning tree (MST) of `G`.

**Proof:** Let `T` be any MST. If `e` is in `T`, we are done. If `e` is not in `T`, then adding `e` to `T` creates a cycle. This cycle must contain at least one other edge `e'` that crosses the cut `(S, V \ S)` (since the cycle starts in `S`, leaves via `e`, and must return to `S`). By the choice of `e` as the minimum-weight crossing edge, `w(e) <= w(e')`. Removing `e'` and adding `e` produces tree `T' = T - {e'} + {e}`. Since `w(e) <= w(e')`, we have `w(T') <= w(T)`. But `T` is an MST, so `w(T') = w(T)`. Thus `e` belongs to the MST `T'`.

### Kruskal Correctness

At each step, Kruskal considers the lightest remaining edge `(u, v)`. Let `S` be the connected component containing `u` in the current forest. If `find(u) != find(v)`, then `v` is in a different component, so `(u, v)` crosses the cut `(S, V \ S)`. Since edges are processed in non-decreasing order, `(u, v)` is the lightest edge crossing this cut (any lighter edge was already processed and would have connected `u` and `v`'s components if it also crossed the cut). By the Cut Property, `(u, v)` belongs to some MST. Adding it is safe.

### Cycle Prevention

The `find(u) != find(v)` check ensures no cycles are formed. If `find(u) == find(v)`, then `u` and `v` are already connected in the current forest, and adding `(u, v)` would create a cycle. Skipping such edges avoids cycles while preserving MST correctness (by the Cut Property, a cycle-forming edge cannot be a cut-crossing edge).

### Complexity

- Sorting edges: O(E log E)
- Union-Find operations: O(E · alpha(V))
- Total: O(E log E)

---

## 4. 0/1 Knapsack DP Correctness — Optimal Substructure

### Problem

Given `n` items, each with weight `w_i` and value `v_i`, and a knapsack of capacity `W`, find the subset of items with total weight <= `W` that maximizes total value.

### DP Formulation

```
dp[i][w] = maximum value achievable with first i items and capacity w

Recurrence:
dp[0][w] = 0  for all w
dp[i][w] = dp[i-1][w]                           if w_i > w
         = max(dp[i-1][w], v_i + dp[i-1][w - w_i])   if w_i <= w
```

### Optimal Substructure

**Claim:** An optimal solution to the problem with `n` items and capacity `W` contains within it an optimal solution to the subproblem with `n-1` items and either capacity `W` (if item `n` is excluded) or capacity `W - w_n` (if item `n` is included).

**Proof:** Let `S` be an optimal subset of items with total weight <= `W`. Consider item `n`:

- **Case 1: `n` not in `S`.** Then `S` is a subset of `{1, ..., n-1}` with total weight <= `W`. If `S` were not optimal for the subproblem with `n-1` items and capacity `W`, there would exist `S'` with higher value and weight <= `W`. But then `S'` would also be a better solution for the original problem, contradicting optimality of `S`.

- **Case 2: `n` in `S`.** Then `S \ {n}` is a subset of `{1, ..., n-1}` with total weight <= `W - w_n`, achieving value `v(S) - v_n`. If this sub-solution were not optimal, there would exist `S'` with higher value and weight <= `W - w_n`. Then `S' ∪ {n}` would have weight <= `W` and higher value than `S`, contradicting optimality of `S`.

Thus, optimal solutions to the original problem are composed of optimal solutions to subproblems.

### Correctness of Recurrence

The recurrence directly encodes the optimal substructure:
- `dp[i][w]` considers both possibilities (include item `i` or exclude it)
- Takes the max, ensuring optimality
- By induction on `i` and `w`, `dp[n][W]` gives the optimal value

### Space Optimization

Since `dp[i][w]` depends only on `dp[i-1][...]`, we can reduce space to O(W) by iterating `w` backwards:

```
for i = 1 to n:
    for w = W down to w_i:
        dp[w] = max(dp[w], v_i + dp[w - w_i])
```

---

## 5. Longest Common Subsequence (LCS) DP Correctness

### Problem

Given strings `X = x_1...x_m` and `Y = y_1...y_n`, find the length of the longest subsequence common to both.

### DP Formulation

```
dp[i][j] = LCS length of prefixes X[1..i] and Y[1..j]

dp[0][j] = dp[i][0] = 0
dp[i][j] = dp[i-1][j-1] + 1                         if x_i == y_j
         = max(dp[i-1][j], dp[i][j-1])               if x_i != y_j
```

### Optimal Substructure

Let `Z = z_1...z_k` be an LCS of `X` and `Y`.

- **If `x_m == y_n`:** Then `z_k = x_m = y_n`, and `Z[1..k-1]` is an LCS of `X[1..m-1]` and `Y[1..n-1]`.
  - *Proof:* If `z_k != x_m`, we could append `x_m` to `Z` and get a longer common subsequence, contradiction. If `Z[1..k-1]` were not an LCS of the prefixes, substituting a longer one would yield a longer common subsequence of `X` and `Y`.

- **If `x_m != y_n`:** Then `z_k != x_m` or `z_k != y_n` (or both). Without loss, if `z_k != x_m`, then `Z` is an LCS of `X[1..m-1]` and `Y[1..n]`.
  - *Proof:* `Z` is a common subsequence of `X[1..m-1]` and `Y` (since it doesn't use `x_m`). If a longer common subsequence of these prefixes existed, it would also be a common subsequence of `X` and `Y`, contradicting `Z` being an LCS.

### Recurrence Correctness

The recurrence covers all three cases:
1. Characters match → both must be included, add 1 to the prefix solution.
2. Characters differ → at most one can be included, take the max of excluding either.
3. Base case (empty prefix) → 0.

By induction on `(i, j)` in lexicographic order, `dp[m][n] = LCS(X, Y)`.

### Reconstruction

To reconstruct the actual subsequence, backtrack from `dp[m][n]`:
- If `x_i == y_j` and `dp[i][j] == dp[i-1][j-1] + 1`: emit `x_i`, move to `(i-1, j-1)`
- Else if `dp[i][j] == dp[i-1][j]`: move to `(i-1, j)`
- Else: move to `(i, j-1)`

---

## 6. Union-Find Amortized Analysis — Inverse Ackermann

### Data Structure

```
MakeSet(x):     parent[x] = x, rank[x] = 0
Find(x):        if parent[x] != x: parent[x] = Find(parent[x]); return parent[x]
Union(x, y):    rx = Find(x), ry = Find(y)
                if rx != ry:
                    if rank[rx] < rank[ry]: swap(rx, ry)
                    parent[ry] = rx
                    if rank[rx] == rank[ry]: rank[rx]++
```

### Union by Rank

**Property:** For any node `x` with `rank[x] = r`, the subtree rooted at `x` has at least `2^r` nodes.

**Proof by induction:** Base case `r = 0`: subtree size 1 = `2^0`. Inductive step: a node of rank `r` is created only by merging two trees whose roots both have rank `r-1`. By induction, each has at least `2^{r-1}` nodes. The merged tree has at least `2^{r-1} + 2^{r-1} = 2^r` nodes.

**Corollary:** `rank[x] <= floor(log n)` for all `x`. There are at most `n / 2^r` nodes of rank `r`.

### Path Compression

`Find(x)` makes every node on the path from `x` to the root point directly to the root. This flattens the tree over time, making future finds faster.

### Amortized Analysis via Inverse Ackermann

The function `alpha(n)` (inverse Ackermann) grows incredibly slowly: `alpha(N) <= 4` for `N < 2^{2^{2^{2^{16}}}}`, which exceeds the number of atoms in the observable universe. For all practical purposes, `alpha(n) <= 5`.

**Theorem [Tarjan 1975]:** A sequence of `m` `MakeSet`, `Union`, and `Find` operations, of which `n` are `MakeSet`, takes O(m · alpha(n)) time in the worst case.

**Amortized cost per operation:** O(alpha(n)).

### Proof Sketch (Potential Method)

Assign each node a potential based on the difference between its rank and the rank of its parent. As path compression flattens the tree, many nodes gain a parent of much higher rank, reducing their potential and paying for the work done during `Find`.

Define the "level" function `log*(n)` (iterated logarithm) and partition ranks into blocks. Each node's rank determines its block. The key insight: a node can be charged for a `Find` only a limited number of times before its parent's rank enters a higher block, at which point a different accounting bucket covers the cost.

The total amortized cost across all operations is bounded by `O(m · alpha(n))`. Since `alpha(n)` is practically constant, Union-Find is essentially O(1) amortized per operation.

---

## 7. Additional Proofs

### Merge Sort Correctness — Divide and Conquer

**Base case:** Array of size 0 or 1 is trivially sorted.
**Inductive step:** Assume `merge_sort` correctly sorts the left and right halves. The `merge` procedure combines two sorted halves into a single sorted array by repeatedly choosing the smaller of the two front elements. This is correct because:
- At each step, the selected element is the smallest remaining overall (since both halves are sorted)
- The merged array preserves the relative order within each half

By mathematical induction on array size, merge sort correctly sorts any array.

### BFS Shortest Path Correctness

**Claim:** For an unweighted graph, BFS visiting nodes in order of discovery computes the shortest path (in number of edges) from `s` to every reachable node.

**Invariant:** Let `d[v]` be the BFS distance label. At all times, `d[v]` is either the true shortest-path distance or an overestimate.

**Key property:** BFS processes nodes in non-decreasing order of distance from `s`. This follows from the FIFO queue discipline, which ensures level `k` nodes are processed before level `k+1` nodes.

**Proof by contradiction:** Suppose `v` is the first node for which `d[v]` is not the true distance. Let `u` be the predecessor of `v` on a true shortest path. Since `u` is closer to `s`, it was processed earlier and `d[u]` is correct. When `u` was processed, edge `(u, v)` was considered. If `v` was not yet discovered, `d[v]` was set to `d[u] + 1`, the true distance. If `v` was already discovered, the FIFO property ensures its distance cannot exceed the true distance. Contradiction.

### Bellman-Ford Correctness

**Claim:** After `k` iterations of the outer loop, `dist[v]` is at most the length of the shortest path from `s` to `v` using at most `k` edges.

**Base case:** `k = 0`. `dist[s] = 0`, all others infinity. Correct.

**Inductive step:** Assume true after `k` iterations. In iteration `k+1`, consider any shortest path of at most `k+1` edges ending at `v` via some predecessor `u`. By induction, after iteration `k`, `dist[u]` is at most the shortest path length to `u` using at most `k` edges. The relaxation of edge `(u, v)` in iteration `k+1` ensures `dist[v] <= dist[u] + w(u,v)`, which is at most the shortest path length to `v` using at most `k+1` edges.

After `|V|-1` iterations, all simple paths (at most `|V|-1` edges) have been considered.

**Negative cycle detection:** Run one more iteration. If any distance decreases, a negative cycle exists, because a simple path in a graph with no negative cycles cannot be improved after `|V|-1` relaxations.
