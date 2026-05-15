# mini-cache

An LRU (Least Recently Used) cache demo combining hash table and doubly linked list for O(1) operations.

## Overview

This demo implements an LRU cache eviction policy — a classic systems problem solved by the elegant composition of two fundamental data structures. The hash table provides O(1) key lookup, while the doubly linked list maintains access order for O(1) eviction of the least recently used item.

## Data Structures

### Hash Table — O(1) Lookup

Uses `hash_table.h` (chaining) to map keys to linked list nodes. Given a key, the hash table returns a direct pointer to the corresponding node in the doubly linked list. This enables:

- `get(key)`: O(1) average — hash lookup yields the node; move node to front of list (mark as most recently used)
- `put(key, value)`: O(1) average — if key exists, update and move to front; if new, prepend to list and insert into hash table

### Doubly Linked List — O(1) Eviction

Uses `linked_list.h` to maintain items in order of access, from most recently used (head) to least recently used (tail). Each node stores `(key, value)` pairs.

- Moving a node to the front after access: O(1) — unlink node, relink at head
- Evicting the least recently used item: O(1) — remove tail node, delete from hash table
- Inserting a new item at head: O(1) — `list_push_front`

### Combined Data Structure

```
Hash Table                    Doubly Linked List
+--------+                   +---+    +---+    +---+
| key1 ->|------------------>| 1 |<-->| 2 |<-->| 3 |
| key2 ->|-------------------| a |    | b |    | c |  (head = MRU, tail = LRU)
| key3 ->|-------------------|   |    |   |    |   |
+--------+                   +---+    +---+    +---+
                              MRU               LRU
```

The hash table values are **pointers to list nodes**, not the data itself. This indirection enables O(1) node relocation without needing to search the list.

## Operations

### `get(key)`

```
1. Hash lookup of key -> node pointer
2. If found:
   a. Remove node from its current position in list (O(1) with prev/next pointers)
   b. Insert node at head (O(1))
   c. Return node->value
3. If not found: return NULL (cache miss)
```

### `put(key, value)`

```
1. Hash lookup of key -> node pointer
2. If found (key exists):
   a. Update node->value
   b. Move node to head of list
3. If not found (new key):
   a. If cache is full (size == capacity):
      - Get tail node (LRU)
      - Remove from hash table using tail->key
      - Remove tail from list
      - Free node
   b. Create new node with (key, value)
   c. Insert at head of list
   d. Insert into hash table: key -> new_node
```

### Complexity Summary

| Operation | Time (Avg) | Time (Worst) |
|---|---|---|
| `get(key)` | O(1) | O(n) — hash collisions |
| `put(key, value)` | O(1) | O(n) — hash collisions |
| Eviction | O(1) | O(1) |
| Space per entry | O(1) | O(1) |

With a well-chosen hash function and load factor <= 0.75, the worst case is extremely unlikely.

## Demo Program

The demo simulates a cache with configurable capacity and a trace of memory accesses.

### Example Run

```
Cache capacity: 4

put(1, "alpha")   -> Cache: [1:alpha]
put(2, "beta")    -> Cache: [2:beta, 1:alpha]
put(3, "gamma")   -> Cache: [3:gamma, 2:beta, 1:alpha]
get(1)            -> Hit: "alpha". Cache: [1:alpha, 3:gamma, 2:beta]
put(4, "delta")   -> Cache: [4:delta, 1:alpha, 3:gamma, 2:beta]
put(5, "epsilon") -> Evict 2:beta. Cache: [5:epsilon, 4:delta, 1:alpha, 3:gamma]
get(2)            -> Miss (evicted)

Hit rate: 1/3 = 33.3%
```

### Input Format

Reads an access trace file:
```
PUT key1 val1
PUT key2 val2
GET key1
PUT key3 val3
...
```

## Build and Run

```bash
make mini-cache
./build/ex_mini-cache [capacity] [trace_file]
```

## Source Files

| File | Purpose |
|---|---|
| `demos/mini-cache/main.c` | Demo entry point, trace parser |
| `demos/mini-cache/cache.c` | LRU cache implementation |
| `demos/mini-cache/cache.h` | LRU cache interface |
| `src/linked_list.c` | Doubly linked list (shared) |
| `src/hash_table.c` | Hash table with chaining (shared) |

## Why LRU? Theoretical Motivation

LRU is optimal among online deterministic caching algorithms up to a constant factor (k/(k-h+1)-competitive, where k is cache size and h is the maximum cache needed by the optimal algorithm). Belady's OPT (clairvoyant) algorithm is optimal but requires knowing the future. LRU approximates OPT using the heuristic that recently used pages are likely to be used again (temporal locality).

## Key Concepts

- **Synergy of data structures:** Neither a hash table nor a doubly linked list alone can achieve O(1) get + O(1) eviction. The combination is an example of using the right tool for each sub-problem.
- **Indirection:** Hash table stores pointers to list nodes, not values, enabling O(1) repositioning.
- **Temporal locality:** LRU exploits the empirical observation that memory accesses exhibit locality of reference.

## References

- CLRS Ch. 10 — Elementary Data Structures (Linked Lists)
- CLRS Ch. 11 — Hash Tables
- Sleator & Tarjan, "Amortized Efficiency of List Update and Paging Rules" (1985)
- MIT 6.006, Recitation 7 — LRU Cache
