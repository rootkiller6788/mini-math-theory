# mini-autocomplete

An autocomplete demo combining trie-based prefix search, sorting with binary search, and heap-based top-K selection.

## Overview

Autocomplete (or "typeahead") is the feature that suggests completions as you type — used in search engines, IDEs, and messaging apps. This demo explores three algorithmic approaches for implementing autocomplete efficiently.

## Approaches

### 1. Trie (Prefix Tree) — O(k) Prefix Search

A **trie** (`trie.h`) is a tree data structure where each node represents a character and paths from root to leaf spell out stored strings. It is the gold standard for prefix-based search.

```
Root
 ├─a
 │  ├─p
 │  │  ├─p──$ ("app")
 │  │  └─e──$ ("ape")
 │  └─r
 │     └─t──$ ("art")
 └─b
    ├─a
    │  └─t──$ ("bat")
    └─e
       └─e──$ ("bee")
```

**Operations:**
- `insert(word)`: O(k) where k = word length. Walk/create nodes for each character, mark terminal.
- `search(word)`: O(k). Walk the trie by characters. Return true if terminal node reached.
- `starts_with(prefix)`: O(p) where p = prefix length. Walk to the prefix node, then **collect all words** in the subtree (DFS from the prefix node).

**Prefix collection:** From the prefix node, perform DFS to collect all terminal descendants. Each collected word takes O(k) to reconstruct (following parent pointers back to root, or building the string during traversal).

**Time complexity for autocomplete:**
- Locate prefix node: O(p)
- Collect completions: O(number of descendant nodes) = O(alphabet_size * average_word_length * num_completions) in worst case

**Space:** O(alphabet_size * total_characters_stored). For a 26-letter alphabet and 1 million words averaging 5 characters, this is roughly 130 million nodes worst-case (though prefix sharing reduces this significantly).

**Use case:** Search engine query suggestions, IDE code completion, contact search.

### 2. Sorting + Binary Search — O(k · log n)

**Approach:**
1. Store all words in a **sorted array** (lexicographic order).
2. To autocomplete prefix `p`:
   - **Binary search** for the first word >= `p` (lower_bound).
   - **Binary search** for the first word > `p + MAX_CHAR` (upper_bound). Since `p + '\xFF'` is lexicographically greater than any string starting with `p`, this finds the end of the range.
   - All words in `[lower, upper)` have `p` as a prefix. Return up to K of them.

**Complexity:**
- Build index: O(n log n) — sort all words once
- Autocomplete query: O(k · log n) for two binary searches + O(K) to collect results

**Advantages over trie:**
- Simpler to implement
- Lower memory overhead (no pointer structures)
- Cache-friendly (contiguous array)

**Disadvantages:**
- Re-sorting required on insertions (or use B-tree)
- Binary search comparisons involve full string comparisons, not just prefix characters

**Use case:** Static dictionaries (e.g., command names in a CLI).

### 3. Heap for Top-K Suggestions — O(k · log n + K log K)

In many applications, completions must be **ranked** — not all completions of a prefix are equally relevant. Search engines rank by query frequency; IDEs rank by recency or context.

**Approach:**
1. Use the trie or sorted array to find all words matching the prefix.
2. Insert matching words into a **min-heap** of size K, keyed by score (frequency, recency, etc.).
3. After processing all candidates, the heap contains the **top-K highest-scoring completions**.
4. Extract in descending order of score (reverse the heap output).

**Complexity:**
- Locate candidates: O(k · log n) or O(p + num_candidates)
- Build top-K heap: O(num_candidates · log K)
- Output: O(K log K) to sort the heap output
- Total: O(k · log n + num_candidates · log K + K log K)

**Alternative — Quickselect:** Use `quickselect` for O(num_candidates) expected time to find the K-th highest score, then partition around it. This avoids the O(log K) factor but is more complex to implement correctly.

## Demo Program

### Commands

```
> add apple 100      (add word with frequency 100)
> add app 50
> add application 200
> add appendix 30
> add banana 80
> suggest "app"      (autocomplete prefix "app")
```

### Output

```
Prefix: "app"
Top 3 suggestions:
  1. application (200)
  2. apple        (100)
  3. app          (50)
(7 candidate nodes examined in trie, 3 heap insertions)
```

### Performance Demo

```
> bench 10000 words, 1000 "app" prefix queries

Trie approach:      0.12 ms/query (collect candidates: 0.10ms, top-K heap: 0.02ms)
Binary search:      0.08 ms/query (2x binary search, K=5)
Heap from sorted:   0.09 ms/query (candidates from sorted range + top-K heap)
```

## Build and Run

```bash
make mini-autocomplete
./build/ex_mini-autocomplete [dictionary_file]
```

## Source Files

| File | Purpose |
|---|---|
| `demos/mini-autocomplete/main.c` | Demo entry point, REPL |
| `demos/mini-autocomplete/autocomplete.c` | Trie + binary search + top-K logic |
| `demos/mini-autocomplete/autocomplete.h` | Autocomplete interface |
| `src/trie.c` | Trie implementation (shared) |
| `src/heap.c` | Min-heap for top-K (shared) |
| `src/search.c` | Binary search, quickselect (shared) |

## Key Concepts

- **Prefix property:** The key insight of autocomplete — all words sharing prefix `p` form a contiguous block both in lexicographic order and in the trie subtree. This makes both the array and trie approaches efficient.
- **Scoring matters:** In practice, the algorithmic challenge shifts from finding completions to ranking them. Frequency-based ranking is the simplest; modern systems use machine-learned ranking with hundreds of features.
- **Trie compression:** Production tries use techniques like **Patricia tries** (path compression — merge single-child chains) and **ternary search trees** to reduce memory.
- **Incremental search:** For interactive use, results for prefix `p` can be refined from results for prefix `p[0..len-1]` rather than recomputed from scratch.

## References

- CLRS Ch. 12 — Binary Search Trees (trie as a specialized tree)
- De La Briandais, "File Searching Using Variable Length Keys" (1959) — original trie paper
- Knuth, *The Art of Computer Programming*, Vol. 3 — Sorting and Searching (Patricia tries)
- MIT 6.006, Lecture 6 — Hashing and Tries
- MIT 6.046J — no direct trie lecture; covered in recitation
