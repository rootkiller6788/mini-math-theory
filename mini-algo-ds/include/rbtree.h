/**
 * Red-Black Tree (Guibas & Sedgewick, 1978) - L8: Advanced Balanced BST
 *
 * Self-balancing BST with color invariants:
 *   1. Every node is red or black
 *   2. Root is black
 *   3. No two consecutive red nodes
 *   4. Every path from root to leaf has same number of black nodes
 *
 * Theorem: Height ≤ 2*log_2(n+1), all operations O(log n)
 *
 * MIT 6.006 / CMU 15-351: Red-Black Trees
 * CLRS Chapter 13: Red-Black Trees
 * Sedgewick "Left-Leaning Red-Black Trees" (2008)
 */

#ifndef RBTREE_H
#define RBTREE_H

#include <stdbool.h>

#define RB_RED   0
#define RB_BLACK 1

typedef struct RBNode {
    int key;
    int value;
    int color;
    struct RBNode* left;
    struct RBNode* right;
    struct RBNode* parent;
} RBNode;

typedef struct {
    RBNode* root;
    RBNode* nil;   /* sentinel NIL node */
    int size;
} RBTree;

RBTree* rbt_create(void);
void rbt_free(RBTree* tree);
void rbt_insert(RBTree* tree, int key, int value);
void rbt_delete(RBTree* tree, int key);
int  rbt_get(RBTree* tree, int key, int default_val);
bool rbt_contains(RBTree* tree, int key);
int  rbt_size(RBTree* tree);
int  rbt_min(RBTree* tree);
int  rbt_max(RBTree* tree);
int  rbt_height(RBTree* tree);
int  rbt_black_height(RBTree* tree);
bool rbt_validate(RBTree* tree);
void rbt_inorder(RBTree* tree, int* keys, int* values, int* idx);
void rbt_print_dot(RBTree* tree, const char* filename);

#endif
