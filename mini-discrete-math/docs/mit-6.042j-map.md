# MIT 6.042J Mathematics for Computer Science — Detailed Topic Map

This document maps every significant topic from MIT 6.042J to our C implementation modules,
including key theorems that are either proven or referenced in the source code.

---

## Unit 1: Proofs → `logic.h` / `logic.c`

| MIT 6.042J Topic | Our C Module | Implementation Detail |
|------------------|--------------|----------------------|
| Propositional Logic (1.1) | `Formula` AST (6 connectives) | `OP_VAR`, `OP_NOT`, `OP_AND`, `OP_OR`, `OP_IMPLIES`, `OP_IFF`, `OP_XOR` |
| Truth Tables (1.2) | `truth_table_print()` | Enumerates all 2^n assignments, evaluates formula tree. |
| Tautology / Contradiction (1.3) | `is_tautology()`, `is_contradiction()` | Exhaustive check over all assignments. |
| Logical Equivalence (1.4) | Implicit — compare truth tables | Two formulas are equivalent iff their truth table columns match. |
| Induction Principle (2.1–2.3) | `proof_by_induction_demo()` | Demonstrates sum 1..n = n(n+1)/2 with base case + inductive step. |

### Key Theorems Covered
- **De Morgan's Laws**: ¬(p ∧ q) ≡ ¬p ∨ ¬q, ¬(p ∨ q) ≡ ¬p ∧ ¬q — can be verified via tautology check on `(p AND q) IFF NOT(NOT p OR NOT q)`.
- **Implication as Disjunction**: p → q ≡ ¬p ∨ q — encoded directly in `OP_IMPLIES` eval.
- **Law of Excluded Middle**: p ∨ ¬p is a tautology — verified by `is_tautology()`.
- **Principle of Mathematical Induction**: If P(1) holds and P(k) ⇒ P(k+1), then P(n) for all n ≥ 1.

---

## Unit 2: Number Theory → `number.h` / `number.c`

| MIT 6.042J Topic | Our C Module | Implementation Detail |
|------------------|--------------|----------------------|
| Divisibility (3.1) | `gcd()` | Iterative Euclidean algorithm. |
| The Division Theorem (3.2) | Implicit in `gcd()`, `mod_*` | a = q·b + r with 0 ≤ r < b. |
| Extended Euclidean (3.3) | `egcd()` | Returns Bézout coefficients (x, y) for ax + by = gcd(a,b). |
| Modular Arithmetic (3.4) | `mod_pow()`, `mod_inverse()` | Fast exponentiation by squaring; modular inverse via egcd. |
| Fermat's Little Theorem (3.5) | `is_prime()` (basic check) | a^(p-1) ≡ 1 (mod p) for prime p when a ⊥ p. |
| Euler's Theorem / Totient (3.5) | `euler_totient()` | φ(n) = n × ∏(1 − 1/p) over distinct primes p dividing n. |
| RSA Cryptosystem (3.7) | `rsa_generate_keys()`, `rsa_encrypt()`, `rsa_decrypt()` | Toy RSA with small primes. |
| Prime Factorization (3.6) | `print_prime_factorization()` | Trial division up to √n. |

### Key Theorems Covered
- **Euclidean Algorithm Correctness**: gcd(a,b) = gcd(b, a mod b) — proven by invariant that any common divisor of a,b is also of b, a mod b.
- **Bézout's Identity**: ∃x,y such that ax + by = gcd(a,b) — computed by `egcd()`.
- **Euler's Theorem**: a^φ(n) ≡ 1 (mod n) when a ⊥ n — basis for RSA correctness proof.
- **RSA Correctness**: m^(ed) ≡ m (mod n) because ed ≡ 1 (mod φ(n)), and by Euler's theorem m^(k·φ(n)+1) ≡ m (mod n).
- **Modular Inverse Uniqueness**: a has an inverse mod m iff gcd(a,m) = 1.

---

## Unit 3: Sets & Relations → `set.h` / `relation.h`

| MIT 6.042J Topic | Our C Module | Implementation Detail |
|------------------|--------------|----------------------|
| Set Basics, Membership (4.1) | `Set` struct, `set_add()`, `set_contains()` | Sorted int array with O(n) search. |
| Set Operations (4.2) | `set_union()`, `set_intersection()`, `set_difference()` | Two-pointer merge for sorted arrays. |
| Subset, Equality (4.2) | `set_subset()`, `set_equal()` | Subset = every element of A is in B. |
| Power Set (4.3) | `power_set_generate()` | Bit-mask enumeration: 2^n subsets. |
| Cartesian Product (4.4) | `cartesian_product()` | All ordered pairs (a,b) for a∈A, b∈B. |
| Binary Relations (4.5) | `Relation` (adjacency matrix) | `relation_add_pair()`, `relation_print_matrix()` |
| Relation Properties (4.6) | `relation_is_reflexive()`, `_symmetric()`, `_transitive()`, `_antisymmetric()` | Matrix check algorithms. |
| Equivalence Relations (4.7) | `relation_is_equivalence()`, `equivalence_classes()` | Partitions domain into equivalence classes. |
| Closures (4.7) | `relation_reflexive_closure()`, `_symmetric_closure()`, `_transitive_closure()` | Warshall's algorithm for transitive (Floyd-Warshall variant). |
| Partial Orders (4.8) | `relation_is_partial_order()`, `hasse_diagram_print()`, `topological_sort()` | Kahn's algorithm for topological ordering. |

### Key Theorems Covered
- **De Morgan's Laws for Sets**: (A ∪ B)ᶜ = Aᶜ ∩ Bᶜ, (A ∩ B)ᶜ = Aᶜ ∪ Bᶜ — can be verified via `set_equal()`.
- **Cardinality of Power Set**: |P(S)| = 2^|S| — verified by bit-mask enumeration (2^n = ps.count).
- **Cardinality of Cartesian Product**: |A × B| = |A| × |B|.
- **Transitive Closure — Warshall's Theorem**: R* = R ∪ R² ∪ R³ ∪ ... — computed in O(n³) by Floyd-Warshall style algorithm.
- **Equivalence Class Partition Theorem**: An equivalence relation on S partitions S into disjoint, nonempty classes.
- **Topological Sort Existence**: A finite poset always has a topological order (Kahn's algorithm guarantees this for any DAG).

---

## Unit 4: Graph Theory → `graph.h` / `graph.c`

| MIT 6.042J Topic | Our C Module | Implementation Detail |
|------------------|--------------|----------------------|
| Graphs, Adjacency (5.1–5.2) | `Graph` with adjacency list | `graph_init()`, `graph_add_edge()` |
| Connectivity (5.3) | `bfs()`, `dfs()`, `has_path()` | BFS/DFS traversal from a start vertex. |
| Walks, Paths, Cycles (5.4) | `has_cycle()` | DFS with 3-state coloring (white/gray/black). |
| Shortest Paths (5.5) | `shortest_path_dijkstra()`, `_bellman_ford()`, `_floyd()` | Dijkstra (non-negative), Bellman-Ford (negative weights allowed), Floyd-Warshall (all-pairs). |
| Bipartite Graphs (5.6) | `is_bipartite()` | 2-coloring via BFS. |
| Graph Coloring (5.7) | `graph_coloring_greedy()` | Greedy sequential coloring. |
| Euler Circuits (5.8) | `has_euler_circuit()`, `find_euler_circuit()` | Hierholzer's algorithm. |
| DAGs (5.9) | `topological_sort_kahn()` | Kahn's indegree-based algorithm. |

### Key Theorems Covered
- **Handshaking Lemma**: ∑ deg(v) = 2|E| for undirected graphs — each edge contributes 2 to the sum of degrees.
- **Euler's Theorem for Circuits**: A connected undirected graph has an Euler circuit iff every vertex has even degree — checked by `has_euler_circuit()`.
- **A Graph is Bipartite iff No Odd Cycles** — verified by 2-coloring BFS in `is_bipartite()`.
- **Dijkstra's Correctness**: For non-negative edge weights, greedy relaxation yields shortest paths (invariant: dist[u] is final when u is extracted).
- **Bellman-Ford Correctness**: After k iterations, distances reflect shortest paths using at most k edges — no negative cycles iff no improvement in n-th iteration.
- **Floyd-Warshall**: DP recurrence `dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j])` for all-pairs shortest paths.

---

## Unit 5: Trees → `tree.h` / `tree.c`

| MIT 6.042J Topic | Our C Module | Implementation Detail |
|------------------|--------------|----------------------|
| Trees, Definitions (6.1) | `TreeNode`, `tree_height()`, `tree_size()` | Recursive tree structure. |
| Tree Traversals (6.2) | `tree_preorder()`, `_inorder()`, `_postorder()`, `_levelorder()` | Pre/in/post/level order traversal to array. |
| Binary Search Trees (6.3) | `bst_insert()`, `bst_search()`, `bst_delete()`, `bst_min()`, `bst_max()` | Standard BST operations. |
| BST Property (6.3) | `tree_is_bst()` | Recursive min/max bound check. |
| AVL Trees (6.4) | `avl_insert()`, `avl_rotate_right()`, `avl_rotate_left()` | Self-balancing with height tracking. |
| Huffman Coding (6.5) | `huffman_build_tree()`, `huffman_generate_codes()`, `_encode()`, `_decode()` | Optimal prefix code construction. |

### Key Theorems Covered
- **Tree Properties**: A tree with n vertices has exactly n−1 edges; any two vertices connected by unique simple path.
- **BST Inorder Traversal Theorem**: In-order traversal of a BST yields keys in sorted (ascending) order.
- **AVL Balance Property**: For any node, |height(left) − height(right)| ≤ 1 ensures O(log n) height.
- **Huffman Coding Optimality** (proof sketch): The optimal prefix code for a given frequency distribution is a full binary tree where the two least-frequent symbols are siblings at maximum depth. By induction, merging them preserves optimality, yielding the Huffman tree as provably optimal.

---

## Unit 6: Counting → `combo.h` / `combo.c`

| MIT 6.042J Topic | Our C Module | Implementation Detail |
|------------------|--------------|----------------------|
| The Sum & Product Rules (8.1) | Implicit base | Foundation for all counting functions. |
| Permutations (8.2) | `permutation(n,r)` = P(n,r) | n!/(n−r)! using incremental multiplication. |
| Combinations (8.3) | `combination(n,r)` = C(n,r) | Multiplicative formula: n·(n−1)···(n−r+1) / r!. |
| Binomial Theorem (8.4) | `binomial_coefficient()`, `pascal_triangle()` | (x + y)^n = ∑ᵢ C(n,i)·xⁱ·yⁿ⁻ⁱ. |
| Permutation Generation (8.5) | `permutations_generate()` | Heap's algorithm — minimal swap sequence. |
| Combination Generation (8.5) | `combinations_generate()` | Recursive backtracking. |
| Stirling Numbers (2nd kind) (8.6) | `stirling_numbers_second()` | S(n,k) = number of ways to partition n labeled items into k nonempty unlabeled subsets. DP recurrence: S(n,k) = S(n−1,k−1) + k·S(n−1,k). |
| Catalan Numbers (8.7) | `catalan_number()` | C_n = C(2n,n)/(n+1). Counts Dyck paths, binary trees, balanced parentheses. |
| Integer Partitions (8.8) | `partitions_of_integer()`, `partitions_enumerate()` | p(n) count; enumeration of all partitions. |
| Derangements (8.9) | `derangements()` | !n = (n−1)(!(n−1) + !(n−2)), the number of permutations with no fixed points. |

### Key Theorems Covered
- **Binomial Theorem**: (x + y)^n = ∑_{k=0}^n C(n,k) x^k y^{n−k} — Pascal's triangle generates the coefficients.
- **Pascal's Identity**: C(n,k) = C(n−1,k−1) + C(n−1,k) — basis of Pascal's triangle construction.
- **Catalan Recurrence**: C_n = ∑_{i=0}^{n−1} C_i·C_{n−1−i}, closed form C_n = C(2n,n)/(n+1).
- **Derangement Recurrence**: !n = (n−1)(!(n−1) + !(n−2)).
- **Stirling Partition Identity**: S(n,k) = S(n−1,k−1) + k·S(n−1,k).
- **Integer Partition Generating Function**: p(0) = 1, p(n) = ∑_{k≥1} (−1)^{k+1} × [p(n − k(3k−1)/2) + p(n − k(3k+1)/2)] (Euler's pentagonal number theorem — not implemented but referenced).

---

## Unit 7: Boolean Algebra (Extension) → `boolalg.h` / `boolalg.c`

| Topic (not in 6.042J, but core for CS) | Our C Module | Implementation Detail |
|----------------------------------------|--------------|----------------------|
| Boolean Functions | `BoolFunction` (function pointer + nvars) | Truth tables for AND, OR, XOR, NAND, NOR, Majority. |
| Standard Forms | `SOP`, `POS`, `sop_from_truth_table()`, `pos_from_truth_table()` | Sum-of-products, Product-of-sums from truth table. |
| Karnaugh Map Simplification | `karnaugh_map_2var/3var/4var()` | K-map simplification for up to 4 variables using prime implicants. |
| Logic Circuits | `LogicCircuit`, `circuit_add_gate()`, `circuit_evaluate()` | Gate-level simulation (Half Adder, Full Adder). |

### Key Theorems Covered
- **Boolean Algebra Axioms**: Commutativity, associativity, distributivity, identity, complement — verified by truth table equality.
- **Absorption Law**: p ∧ (p ∨ q) ≡ p, p ∨ (p ∧ q) ≡ p.
- **De Morgan's Laws** (Boolean form): ¬(p ∧ q) ≡ ¬p ∨ ¬q, ¬(p ∨ q) ≡ ¬p ∧ ¬q.
- **Shannon's Expansion Theorem**: f(x₁,…,x_n) = x_i·f(x_i=1) + ¬x_i·f(x_i=0) — basis for circuit synthesis.
- **Every Boolean Function has SOP/POS**: Any truth table can be expressed as sum of minterms or product of maxterms.

---

## Cross-Reference: MIT 6.042J Lecture Schedule vs Our Modules

| MIT Lecture | Title | Module(s) Used |
|-------------|-------|----------------|
| L01 | Intro, Proofs | `logic` |
| L02 | Induction | `logic` (induction demo) |
| L03–L04 | Number Theory I–II | `number` |
| L05–L06 | Sets, Relations | `set`, `relation` |
| L07–L08 | Graph Theory I–II | `graph` |
| L09–L10 | Trees, Sums, Asymptotics | `tree`, `combo` |
| L11–L12 | Counting I–II | `combo` |
| L13–L14 | Generating Functions, Probability | `combo` (partial) |
| L15–L16 | Recurrences, Eulerian Walk | `graph` (Euler) |

---

## Extended Reading: MIT 6.045J Connections

| 6.045J Topic | Our Module | Connection |
|--------------|------------|------------|
| DFA/NFA State Minimization | `set`, `relation` | Equivalence relations partition states. |
| Pumping Lemma for Regular Languages | `logic` | Uses counting (pigeonhole principle). |
| Context-Free Grammars | `tree` | Parse trees are tree structures. |
| NP-Completeness | `graph`, `combo` | Graph coloring, Hamiltonian path are NP-complete. |
