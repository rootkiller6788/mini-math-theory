# mini-index

A simple database index demo exploring BST-based ordered key lookup and the path to disk-optimized B-Trees.

## Overview

Databases use indexes to find rows by key without scanning the entire table. This demo builds a simplified index using a Binary Search Tree and explains how the concept scales to B-Trees — the data structure underlying virtually every production database (MySQL InnoDB, PostgreSQL, SQLite, MongoDB).

## Index Structures

### BST Index — In-Memory Ordered Key Lookup

A **Binary Search Tree** (`bst.h`) stores `(key, record_pointer)` pairs ordered by key. This enables:

- **Point query:** `SELECT * FROM users WHERE id = 42` — O(log n) BST search
- **Range query:** `SELECT * FROM users WHERE id BETWEEN 100 AND 200` — O(log n + k) where k is the number of matching records (in-order traversal)
- **Ordered scan:** `SELECT * FROM users ORDER BY id` — O(n) in-order traversal

**Why BST over Hash Table?** Hash tables give O(1) point queries but cannot efficiently support range queries or ordered scans. BSTs preserve key order, enabling `lower_bound` / `upper_bound` operations.

### Demo Walkthrough

```
Insert: (42, "Alice"), (17, "Bob"), (73, "Charlie"), (5, "Diana"), (88, "Eve")

BST Structure:
        42
       /  \
     17    73
    /        \
   5          88

Range query [10, 80]:
  - In-order traversal starting from lower_bound(10)
  - Returns: (17, "Bob"), (42, "Alice"), (73, "Charlie")

Point query 42:
  - Walk: root(42) matches -> return "Alice"
```

### From BST to B-Tree — The Disk I/O Gap

#### The Problem with BSTs on Disk

A BST with 1 billion records has a height of approximately 30 (assuming balance). Each traversal step potentially requires a **disk seek**. At ~10ms per random I/O, a point query takes ~300ms — unacceptable.

The fundamental insight: **CPU operations are essentially free compared to disk I/O.** The optimization goal is to minimize the number of disk blocks read.

#### B-Tree Solution

A **B-Tree** (`btree.h`) generalizes the BST by storing **many keys per node** (branching factor B, typically 50–2000):

```
                [100 | 200 | 300]       -- root node (4 keys, 5 children)
               /      |      |      \
    [30|60|90]   [130|160|190]   [230|260|290]   [330|360|390]
   /  /  /  \    /  /  /   \     /  /  /   \     /  /  /   \
  ...leaves...

Each node = ONE disk block (typically 4KB–16KB)
```

**Properties:**
- All leaves are at the same depth (perfect balance)
- Each internal node (except root) has between `ceil(B/2)` and `B` children
- Height for 1 billion keys with B=100: `log_100(10^9) ≈ 4.5` — only **4–5 disk seeks** per query
- A range query reads leaf nodes sequentially (fast sequential I/O)

#### Comparison

| Property | BST | B-Tree (B=100) |
|---|---|---|
| Keys per node | 1 | 50–100 |
| Height for 10^9 records | ~30 | ~5 |
| Disk seeks per point query | ~30 | ~5 |
| Range query | O(log n + k) random I/O | O(log_B n + k/B) mostly sequential I/O |
| Memory/disk | In-memory only | Disk-optimized |

### Implementation Notes

The demo implements a **B-Tree variant** with:
- **Internal nodes:** Store keys and child pointers
- **Leaf nodes:** Store `(key, record_pointer)` pairs
- **Splitting:** When a node overflows (> B keys), the median key is promoted to the parent
- **Merging/Redistribution:** When a node underflows (< ceil(B/2) keys) after deletion

## Demo Program

### Commands

```
CREATE users id:integer name:string
INSERT users (1, "Alice")
INSERT users (2, "Bob")
SELECT users WHERE id = 1
SELECT users WHERE id BETWEEN 10 AND 20
DELETE users WHERE id = 1
```

### Output

```
> CREATE users id:integer name:string
Index created on users.id

> INSERT users (1, "Alice")
Inserted. BST height: 1, nodes: 1

> INSERT users (2, "Bob")
Node split at key 2 (overflow). BST height: 2, nodes: 2

> SELECT users WHERE id = 1
[1] Alice (1 disk read simulated)

> SELECT users WHERE id BETWEEN 10 AND 20
(in-order traversal from lower_bound)
[10] ...
... (found in sequential leaf scan)
```

## Build and Run

```bash
make mini-index
./build/ex_mini-index
```

## Source Files

| File | Purpose |
|---|---|
| `demos/mini-index/main.c` | Demo entry point, REPL |
| `demos/mini-index/index.c` | Index implementation (BST + B-Tree comparison) |
| `demos/mini-index/index.h` | Index interface |
| `src/bst.c` | Binary Search Tree (shared) |
| `src/btree.c` | B-Tree implementation (shared) |

## Key Concepts

- **Index as auxiliary structure:** An index is a separate data structure that maps keys to record locations. It is maintained alongside the actual data.
- **Clustered vs unclustered:** A clustered index stores actual records in leaf nodes (MySQL InnoDB primary key). An unclustered index stores pointers to heap records (PostgreSQL).
- **Fanout matters:** The branching factor B is chosen so that one node fits exactly in one disk page. Larger B = shallower tree = fewer I/Os.
- **Write amplification:** B-Tree inserts may cascade splits up to the root. In the worst case, O(log_B n) nodes are written, but amortized it's much less.

## References

- CLRS Ch. 12 — Binary Search Trees
- CLRS Ch. 18 — B-Trees
- Bayer & McCreight, "Organization and Maintenance of Large Ordered Indexes" (1972)
- MySQL Internals Manual — InnoDB Index Structure
- MIT 6.006, Lecture 6 — AVL Trees (balanced search trees)
- MIT 6.046J, Lecture 9 — B-Trees
