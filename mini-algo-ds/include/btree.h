/**
 * B-Tree (Bayer & McCreight, 1972) - L4: Database Index Structure
 *
 * Balanced m-way search tree optimized for disk I/O.
 * Each node contains k keys with k+1 children, where t-1 <= k <= 2t-1.
 * 
 * Theorem: B-Tree of order t stores n keys with height h <= log_t((n+1)/2)
 * All operations: O(log_t n) = O(log n) disk accesses.
 *
 * MIT 6.046J / CMU 15-445: B+Tree indexes
 * CLRS Chapter 18: B-Trees
 */

#ifndef BTREE_H
#define BTREE_H

#include <stdbool.h>

/* Minimum degree t >= 2. Max keys per node = 2*t - 1 */
#define BTREE_MIN_DEGREE 3
#define BTREE_MAX_KEYS (2 * BTREE_MIN_DEGREE - 1)

typedef struct BTreeNode {
    int keys[BTREE_MAX_KEYS];
    struct BTreeNode* children[BTREE_MAX_KEYS + 1];
    int nkeys;
    bool is_leaf;
} BTreeNode;

typedef struct {
    BTreeNode* root;
    int t;       /* minimum degree */
    int size;
} BTree;

BTree* btree_create(void);
void btree_free(BTree* tree);
bool btree_search(BTree* tree, int key);
void btree_insert(BTree* tree, int key);
void btree_delete(BTree* tree, int key);
int  btree_size(BTree* tree);
int  btree_height(BTree* tree);
void btree_traverse_inorder(BTree* tree, int* arr, int* idx);
int  btree_range_count(BTree* tree, int lo, int hi);
int  btree_range_query(BTree* tree, int lo, int hi, int* arr);
int  btree_min(BTree* tree);
int  btree_max(BTree* tree);
bool btree_validate(BTree* tree);
void btree_print_dot(BTree* tree, const char* filename);

#endif
