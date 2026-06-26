/**
 * AVL Tree (Adelson-Velsky & Landis, 1962)
 * L4: Standard Theorem - Height Balance Invariant: |bf| <= 1 ensures O(log n) height
 * L3: Engineering Structure - Self-balancing BST with rotation operations
 *
 * Theorem: For AVL tree with n nodes, height h satisfies:
 *   log_2(n+1) <= h <= 1.44 * log_2(n+2) - 0.328
 *
 * MIT 6.006 Lecture 6: AVL Trees, AVL Sort
 * CLRS Chapter 13 (Red-Black) / Goodrich & Tamassia Chapter 11
 */

#ifndef AVL_H
#define AVL_H

#include <stdbool.h>

typedef struct AVLNode {
    int key;
    int value;
    int height;
    struct AVLNode* left;
    struct AVLNode* right;
} AVLNode;

typedef struct {
    AVLNode* root;
    int size;
} AVL;

AVL* avl_create(void);
void avl_free(AVL* tree);
void avl_insert(AVL* tree, int key, int value);
void avl_delete(AVL* tree, int key);
int  avl_get(AVL* tree, int key, int default_val);
bool avl_contains(AVL* tree, int key);
int  avl_size(AVL* tree);
int  avl_height(AVL* tree);
bool avl_is_balanced(AVL* tree);
int  avl_min(AVL* tree);
int  avl_max(AVL* tree);
int  avl_floor(AVL* tree, int key);
int  avl_ceil(AVL* tree, int key);
int  avl_select(AVL* tree, int k);
int  avl_rank(AVL* tree, int key);
int  avl_range_count(AVL* tree, int lo, int hi);
int  avl_range_query(AVL* tree, int lo, int hi, int* keys, int* values);
void avl_inorder(AVL* tree, int* keys, int* values, int* idx);
void avl_preorder(AVL* tree, int* keys, int* values, int* idx);
void avl_postorder(AVL* tree, int* keys, int* values, int* idx);
void avl_level_order(AVL* tree, int* keys, int* values, int* idx);
AVLNode* avl_rotate_right(AVLNode* y);
AVLNode* avl_rotate_left(AVLNode* x);

#endif
