/**
 * Skip List (Pugh, 1990) - L8: Advanced Probabilistic Data Structure
 *
 * Probabilistic alternative to balanced trees with O(log n) expected time.
 * Uses randomized level promotion with coin-flip geometry.
 *
 * Theorem: E[level of element] = 2, E[list height] = O(log n)
 * Each operation (search/insert/delete) is O(log n) expected.
 *
 * MIT 6.046J Lecture 8: Randomization, Skip Lists
 * CMU 15-451: Algorithm Design and Analysis
 */

#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <stdbool.h>

#define SKIPLIST_MAX_LEVEL 16

typedef struct SkipNode {
    int key;
    int value;
    struct SkipNode** forward;
    int level;
} SkipNode;

typedef struct {
    SkipNode* header;
    int level;
    int size;
    int max_level;
} SkipList;

SkipList* sl_create(int max_level);
void sl_free(SkipList* sl);
void sl_insert(SkipList* sl, int key, int value);
bool sl_delete(SkipList* sl, int key);
int  sl_search(SkipList* sl, int key, int default_val);
bool sl_contains(SkipList* sl, int key);
int  sl_size(SkipList* sl);
int  sl_min(SkipList* sl);
int  sl_max(SkipList* sl);
void sl_range_query(SkipList* sl, int lo, int hi, int* keys, int* values, int* count);
void sl_print(SkipList* sl);

#endif
