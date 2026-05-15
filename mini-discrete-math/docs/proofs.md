# Key Proofs — Implemented or Referenced

This document collects the mathematical proofs behind the algorithms in our C modules.
Each proof is either directly demonstrated by code or referenced as the theoretical
foundation for a function.

---

## 1. Sum of First n Integers (Mathematical Induction)

**Theorem**: For all n ≥ 1, 1 + 2 + ... + n = n(n+1)/2.

**Proof (by induction on n)**:

**Base case** (n = 1):
- LHS = 1
- RHS = 1·2/2 = 1 ✓

**Inductive hypothesis**: Assume P(k) holds for some k ≥ 1:
  1 + 2 + ... + k = k(k+1)/2

**Inductive step**: Prove P(k+1):
  1 + 2 + ... + k + (k+1)
  = k(k+1)/2 + (k+1)          [by inductive hypothesis]
  = (k+1)(k/2 + 1)
  = (k+1)(k+2)/2
  = (k+1)((k+1)+1)/2 ✓

By induction, P(n) holds for all n ≥ 1. ∎

**Code reference**: `proof_by_induction_demo()` in `src/logic.c:116` prints the base case, inductive step, and verifies for n ≤ 10.

---

## 2. GCD Correctness (Euclidean Algorithm)

**Theorem**: For integers a ≥ b > 0, gcd(a,b) = gcd(b, a mod b).

**Proof**:

Let d = gcd(a,b). Then d|a and d|b.

Let r = a mod b, so a = q·b + r for some integer q, where 0 ≤ r < b.

Since d|a and d|b, we have d|(a − q·b) = r. So d is a common divisor of b and r.

Conversely, let d' = gcd(b,r). Then d'|b and d'|r. Since a = q·b + r, d'|a. So d' is a common divisor of a and b.

Since d|b, d|r and any common divisor of b,r divides both, gcd(a,b) = gcd(b,r). ∎

**Termination**: b strictly decreases each iteration (since r < b), eventually reaching 0. When b = 0, gcd(a,0) = a.

**Code reference**: `gcd()` in `src/number.c:6` implements the iterative version:
```
while (b) { t = b; b = a % b; a = t; } return a;
```

---

## 3. Bézout's Identity (Extended Euclidean Correctness)

**Theorem**: For any integers a, b, there exist integers x, y such that ax + by = gcd(a,b).

**Proof (by strong induction on b)**:

**Base case** (b = 0): gcd(a,0) = a. Choose x = 1, y = 0: a·1 + 0·0 = a = gcd(a,0). ✓

**Inductive step**: Assume the identity holds for (b, a mod b):
  ∃x₁, y₁ such that b·x₁ + (a mod b)·y₁ = gcd(b, a mod b)

Let a = q·b + r where r = a mod b, so r = a − q·b.

Then: b·x₁ + (a − q·b)·y₁ = gcd(a,b)  [since gcd(a,b) = gcd(b, a mod b)]

Rearranging: a·y₁ + b·(x₁ − q·y₁) = gcd(a,b)

So let x = y₁, y = x₁ − ⌊a/b⌋·y₁, giving ax + by = gcd(a,b). ∎

**Code reference**: `egcd()` in `src/number.c:15` recursively applies this identity.

---

## 4. RSA Correctness (Euler's Theorem)

**Theorem**: For RSA with primes p, q, n = pq, φ(n) = (p−1)(q−1), e·d ≡ 1 (mod φ(n)):
  (mᵉ)ᵈ ≡ m (mod n) for all messages m where 0 ≤ m < n.

**Proof**:

We need to show m^(ed) ≡ m (mod n).

Since ed ≡ 1 (mod φ(n)), ∃k such that ed = k·φ(n) + 1.

So m^(ed) = m^(k·φ(n) + 1) = m · (m^φ(n))^k.

**Case 1**: gcd(m, n) = 1 (m is coprime to n).
  By Euler's theorem: m^φ(n) ≡ 1 (mod n).
  Therefore m^(ed) ≡ m · 1^k ≡ m (mod n). ✓

**Case 2**: m is a multiple of p (or q, symmetric).
  If p|m, then m ≡ 0 (mod p), so m^(ed) ≡ 0 ≡ m (mod p).
  Since gcd(m, n) ≠ 1, m is not coprime to n. However, by the Chinese Remainder Theorem,
  it suffices to show m^(ed) ≡ m (mod p) and (mod q).
  
  If p|m but q∤m, then m^(ed) ≡ m (mod p) trivially, and by case 1 applied mod q:
  m^(q−1) ≡ 1 (mod q) ⇒ m^(ed) ≡ m · (m^(q−1))^(k(p−1)) ≡ m · 1 ≡ m (mod q).
  
  By CRT, m^(ed) ≡ m (mod pq = n). ✓

**Code reference**: `rsa_encrypt()`/`rsa_decrypt()` in `src/number.c:80-86`, with key generation at `src/number.c:69`.

**Test verification**: Using p=61, q=53, encrypt and decrypt a known message — must roundtrip exactly.

---

## 5. Set Identity Proofs (De Morgan's Laws)

**Theorem**: For subsets A, B of a universal set U:
  (1) (A ∪ B)ᶜ = Aᶜ ∩ Bᶜ
  (2) (A ∩ B)ᶜ = Aᶜ ∪ Bᶜ

**Proof of (1)**:

x ∈ (A ∪ B)ᶜ
⇔ x ∉ (A ∪ B)
⇔ ¬(x ∈ A ∨ x ∈ B)
⇔ (x ∉ A) ∧ (x ∉ B)           [De Morgan's law for propositions]
⇔ x ∈ Aᶜ ∧ x ∈ Bᶜ
⇔ x ∈ (Aᶜ ∩ Bᶜ)

Each step is ⇔, establishing set equality. Proof of (2) is symmetric. ∎

**Code reference**: The identities can be verified using `set_equal()`:
```c
Set complement_union = set_difference(&universe, &union_AB);
Set inter_complements = set_intersection(&complement_A, &complement_B);
assert(set_equal(&complement_union, &inter_complements));
```

---

## 6. Graph Handshaking Lemma

**Theorem**: In any undirected graph G = (V,E), ∑_{v∈V} deg(v) = 2|E|.

**Proof**:

Count the number of (vertex, incident edge) pairs in two ways:
- Method 1: For each vertex v, count its incident edges → deg(v). Sum: ∑ deg(v).
- Method 2: Each edge e = {u,v} contributes exactly 2 to the sum (one for u, one for v). Total: 2·|E|.

Since both methods count the same set of (v,e) pairs, ∑ deg(v) = 2|E|. ∎

**Corollary**: The number of vertices with odd degree is always even.

**Code reference**: The handshaking lemma can be verified by summing degrees from the adjacency list:
```c
int sum_degrees = 0;
for (int v = 0; v < g->nvertices; v++)
    for (EdgeNode* e = g->adj[v]; e; e = e->next)
        sum_degrees++;
// sum_degrees == 2 * g->nedges (for undirected)
```

---

## 7. Euler Circuit Existence Theorem

**Theorem**: A connected undirected graph has an Euler circuit iff every vertex has even degree.

**Proof (⇒)**: If an Euler circuit exists, each time the circuit enters a vertex, it must leave via a different edge. Thus, every visit consumes two distinct edges incident to that vertex. Since the circuit uses every edge exactly once, the degree of each vertex must be even.

**Proof (⇐)**: If all vertices have even degree and the graph is connected, an Euler circuit can be constructed using Hierholzer's algorithm:
1. Start from any vertex, follow unused edges until returning to start (possible because all degrees are even, so no "dead end" except the start vertex).
2. If unused edges remain, find a vertex on the current circuit with unused edges and repeat step 1, splicing the new cycle into the existing circuit.
3. Continue until all edges are used.

The algorithm always terminates because each step uses previously unused edges, and the total number of edges is finite. ∎

**Code reference**: `has_euler_circuit()` (degree check) and `find_euler_circuit()` (Hierholzer) in `src/graph.c:236-285`.

---

## 8. Catalan Number Formula (Proof Sketch)

**Theorem**: The n-th Catalan number C_n = (1/(n+1))·C(2n,n) counts:
- Valid parentheses sequences of n pairs
- Rooted binary trees with n+1 leaves
- Dyck paths of length 2n (paths from (0,0) to (2n,0) staying above x-axis)

**Proof sketch (via Dyck paths)**:

Total unrestricted paths from (0,0) to (2n,0) with steps (1,1) and (1,−1): C(2n,n).

Let B be the set of "bad" paths (those that go below the x-axis). For each bad path, find the first step where it hits y = −1. Reflect the remaining steps (swap up/down). This gives a bijection from bad paths to paths from (0,0) to (2n,−2). The latter has C(2n,n+1) paths.

Therefore: C_n = C(2n,n) − C(2n,n+1)
              = C(2n,n) − (n/(n+1))·C(2n,n)
              = (1/(n+1))·C(2n,n). ∎

**Code reference**: `catalan_number()` in `src/combo.c:93`:
```c
return combination(2 * n, n) / (n + 1);
```

---

## 9. Huffman Coding Optimality (Proof Sketch)

**Theorem**: Among all prefix-free codes for a given frequency distribution, the Huffman code minimizes the expected encoding length.

**Proof (by induction on the number of distinct symbols)**:

**Base case** (k = 2): Only two symbols exist. The only optimal prefix code assigns 0 to one and 1 to the other. The Huffman algorithm merges them and assigns 0 and 1 as the final branch, matching this. ✓

**Inductive step**: Assume Huffman is optimal for k−1 symbols.

**Lemma 1** (Greedy choice): The two least-frequent symbols can be placed as siblings at the deepest level of an optimal tree. (Suppose not: swapping a deeper, more-frequent symbol with a shallower, less-frequent one strictly decreases expected length, contradicting optimality.)

**Lemma 2** (Optimal substructure): After merging the two least-frequent symbols into a meta-symbol with frequency f₁+f₂, an optimal tree for the (k−1)-symbol problem corresponds to an optimal tree for the original k-symbol problem (by expanding the merged node).

By induction hypothesis, Huffman computes an optimal tree for the (k−1)-symbol problem. By Lemma 2, expanding the merged node yields an optimal tree for k symbols. ∎

**Code reference**: `huffman_build_tree()` in `src/tree.c:191` implements the greedy merging algorithm.

---

## 10. Transitive Closure Correctness (Warshall's Algorithm)

**Theorem**: Warshall's algorithm computes the transitive closure R* of a binary relation R on a set of size n.

**Algorithm** (Floyd-Warshall adaptation for Boolean matrices):
```
for k = 0 to n-1:
    for i = 0 to n-1:
        for j = 0 to n-1:
            R[i][j] = R[i][j] ∨ (R[i][k] ∧ R[k][j])
```

**Proof (by invariant)**:

Let R⁽ᵏ⁾[i][j] be true iff there exists a path from i to j using only intermediate vertices numbered < k.

Base: R⁽⁰⁾[i][j] = R[i][j] (no intermediates allowed beyond direct edges).

Inductive step at iteration k:
  R⁽ᵏ⁺¹⁾[i][j] is true iff:
  - There was already a path using intermediates < k, OR
  - There is a path from i to k using intermediates < k AND from k to j using intermediates < k.

This corresponds exactly to: R⁽ᵏ⁾[i][j] ∨ (R⁽ᵏ⁾[i][k] ∧ R⁽ᵏ⁾[k][j]).

After n iterations, R⁽ⁿ⁾ = R*. ∎

**Code reference**: `relation_transitive_closure()` in `src/relation.c:100` implements Warshall's algorithm.

---

## 11. Topological Sort Correctness (Kahn's Algorithm)

**Theorem**: For a finite directed acyclic graph (DAG), Kahn's algorithm produces a linear ordering where for every directed edge (u,v), u appears before v.

**Proof**:

**Algorithm**: Repeatedly remove a vertex with indegree 0 and append it to the output order, decrementing the indegree of its successors.

**Invariant**: At each step, the remaining subgraph is a DAG.
- Initially, the graph is a DAG by hypothesis.
- Removing a vertex with indegree 0 cannot create a cycle (no edge points to it).

**Termination**: In a finite DAG, there is always at least one vertex with indegree 0 (otherwise, following predecessors would eventually repeat a vertex, creating a cycle). So the algorithm can always progress until all vertices are removed.

**Correctness**: When edge (u,v) exists, u must be removed before v (since v initially has indegree > 0, and u contributes to v's indegree. v's indegree only reaches 0 after u is removed). So u appears before v in the output order. ∎

**Code references**:
- Graph-based: `topological_sort_kahn()` in `src/graph.c:149`
- Relation-based: `topological_sort()` in `src/relation.c:155` (same algorithm on relation matrix)

---

## 12. Dijkstra's Algorithm Correctness

**Theorem**: For a weighted graph with non-negative edge weights, Dijkstra's algorithm computes shortest paths from a source vertex to all other vertices.

**Proof (by loop invariant)**:

**Invariant**: At the start of each iteration of the main loop, for each vertex v:
- If v is in the "visited" set, dist[v] is the true shortest distance from source to v.
- If v is not visited, dist[v] is the shortest distance among paths that use only visited vertices as intermediates (except possibly the last edge).

**Initialization**: dist[source] = 0, all others = ∞. Visited set is empty. Invariant holds.

**Maintenance**: Extract u with minimum dist among unvisited vertices. Claim: dist[u] is the true shortest distance to u.
- Suppose not. Then there exists a shorter path P to u via some unvisited vertex x. But the weight of P's prefix to x is at least dist[x] (by invariant), and dist[x] ≥ dist[u] (since u was chosen as minimum). Adding non-negative edge weights preserves the inequality, contradicting that P is shorter.

After adding u to visited, relax edges (u,v): dist[v] = min(dist[v], dist[u] + w(u,v)). This maintains the invariant.

**Termination**: When all vertices are visited, dist[v] is the shortest distance from source to v for all v. ∎

**Code reference**: `shortest_path_dijkstra()` in `src/graph.c:86`.

---

## 13. Bipartite Graph Characterization

**Theorem**: A graph is bipartite iff it contains no odd-length cycle.

**Proof (⇒)**: If a graph is bipartite with partition (L,R), any path alternates between L and R. A cycle must therefore have even length (returns to same part after alternating an even number of times).

**Proof (⇐)**: If a graph has no odd cycle, run BFS 2-coloring: assign color 0 to the start vertex, then for each neighbor, assign the opposite color. If a conflict is found (two adjacent vertices have the same color), then there exists an odd cycle (the path between them in the BFS tree plus the conflicting edge). Since no odd cycle exists, the 2-coloring succeeds, giving a bipartition. ∎

**Code reference**: `is_bipartite()` in `src/graph.c:194`.

---

## 14. Stirling Number Recurrence (Second Kind)

**Theorem**: S(n,k) = S(n−1,k−1) + k·S(n−1,k), where S(n,k) is the number of ways to partition n labeled items into k nonempty unlabeled subsets.

**Proof**:

Consider element n of the set {1,...,n}. In any partition into k blocks:
- **Case 1**: Element n is in a singleton block {n}. The remaining n−1 elements must be partitioned into k−1 blocks: S(n−1,k−1) ways.
- **Case 2**: Element n is in a block with other elements. First partition the remaining n−1 elements into k blocks: S(n−1,k) ways. Then choose which of the k blocks to place element n in: k choices.

Both cases are disjoint and exhaustive. Summing gives the recurrence. ∎

**Base cases**: S(0,0) = 1; S(n,0) = 0 for n > 0; S(0,k) = 0 for k > 0.

**Code reference**: `stirling_numbers_second()` in `src/combo.c:83` uses DP with this recurrence.
