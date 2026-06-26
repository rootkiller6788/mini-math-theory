/**
 * AVL Tree Implementation
 *
 * Self-balancing BST with height-balance invariant |bf| <= 1.
 * Four rotation cases: LL, RR, LR, RL.
 *
 * Height bound: h <= 1.44 * log_2(n+2) - 0.328
 * Proof via Fibonacci trees: min nodes N(h) = N(h-1)+N(h-2)+1
 */

#include "avl.h"
#include <stdlib.h>

static int max_int(int a, int b) { return a > b ? a : b; }

static AVLNode* node_create(int key, int value) {
    AVLNode* n = (AVLNode*)malloc(sizeof(AVLNode));
    n->key = key;
    n->value = value;
    n->height = 1;
    n->left = NULL;
    n->right = NULL;
    return n;
}

static int node_height(AVLNode* n) {
    return n ? n->height : 0;
}

static void node_update_height(AVLNode* n) {
    n->height = 1 + max_int(node_height(n->left), node_height(n->right));
}

static int balance_factor(AVLNode* n) {
    return n ? node_height(n->left) - node_height(n->right) : 0;
}

/* L4 Standard: AVL Rotation operations */

AVLNode* avl_rotate_right(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    node_update_height(y);
    node_update_height(x);
    return x;
}

AVLNode* avl_rotate_left(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    node_update_height(x);
    node_update_height(y);
    return y;
}

static AVLNode* rebalance(AVLNode* node) {
    node_update_height(node);
    int bf = balance_factor(node);
    if (bf > 1 && balance_factor(node->left) >= 0)
        return avl_rotate_right(node);
    if (bf > 1 && balance_factor(node->left) < 0) {
        node->left = avl_rotate_left(node->left);
        return avl_rotate_right(node);
    }
    if (bf < -1 && balance_factor(node->right) <= 0)
        return avl_rotate_left(node);
    if (bf < -1 && balance_factor(node->right) > 0) {
        node->right = avl_rotate_right(node->right);
        return avl_rotate_left(node);
    }
    return node;
}

/* Core API */

AVL* avl_create(void) {
    AVL* tree = (AVL*)malloc(sizeof(AVL));
    tree->root = NULL;
    tree->size = 0;
    return tree;
}

static void free_nodes(AVLNode* n) {
    if (!n) return;
    free_nodes(n->left);
    free_nodes(n->right);
    free(n);
}

void avl_free(AVL* tree) {
    free_nodes(tree->root);
    free(tree);
}

static AVLNode* insert_rec(AVLNode* node, int key, int value, int* added) {
    if (!node) { *added = 1; return node_create(key, value); }
    if (key < node->key)
        node->left = insert_rec(node->left, key, value, added);
    else if (key > node->key)
        node->right = insert_rec(node->right, key, value, added);
    else { node->value = value; return node; }
    return rebalance(node);
}

void avl_insert(AVL* tree, int key, int value) {
    int added = 0;
    tree->root = insert_rec(tree->root, key, value, &added);
    tree->size += added;
}

static AVLNode* find_min(AVLNode* node) {
    while (node && node->left) node = node->left;
    return node;
}

static AVLNode* find_max(AVLNode* node) {
    while (node && node->right) node = node->right;
    return node;
}

static AVLNode* delete_rec(AVLNode* node, int key, int* deleted) {
    if (!node) return NULL;
    if (key < node->key)
        node->left = delete_rec(node->left, key, deleted);
    else if (key > node->key)
        node->right = delete_rec(node->right, key, deleted);
    else {
        *deleted = 1;
        if (!node->left) { AVLNode* r = node->right; free(node); return r; }
        if (!node->right) { AVLNode* l = node->left; free(node); return l; }
        AVLNode* succ = find_min(node->right);
        node->key = succ->key;
        node->value = succ->value;
        int dummy = 0;
        node->right = delete_rec(node->right, succ->key, &dummy);
    }
    return rebalance(node);
}

void avl_delete(AVL* tree, int key) {
    int deleted = 0;
    tree->root = delete_rec(tree->root, key, &deleted);
    tree->size -= deleted;
}

static int get_rec(AVLNode* node, int key, int def) {
    if (!node) return def;
    if (key < node->key) return get_rec(node->left, key, def);
    if (key > node->key) return get_rec(node->right, key, def);
    return node->value;
}

int avl_get(AVL* tree, int key, int default_val) {
    return get_rec(tree->root, key, default_val);
}

static bool contains_rec(AVLNode* node, int key) {
    if (!node) return false;
    if (key == node->key) return true;
    return contains_rec(key < node->key ? node->left : node->right, key);
}

bool avl_contains(AVL* tree, int key) {
    return contains_rec(tree->root, key);
}

int avl_size(AVL* tree) { return tree->size; }
int avl_height(AVL* tree) { return node_height(tree->root); }

static bool check_balanced(AVLNode* n) {
    if (!n) return true;
    if (abs(balance_factor(n)) > 1) return false;
    return check_balanced(n->left) && check_balanced(n->right);
}

bool avl_is_balanced(AVL* tree) {
    return check_balanced(tree->root);
}

/* Ordered operations */

int avl_min(AVL* tree) {
    AVLNode* n = find_min(tree->root);
    return n ? n->key : -1;
}

int avl_max(AVL* tree) {
    AVLNode* n = find_max(tree->root);
    return n ? n->key : -1;
}

static int floor_rec(AVLNode* node, int key, int best) {
    if (!node) return best;
    if (node->key == key) return key;
    if (node->key < key) return floor_rec(node->right, key, node->key);
    return floor_rec(node->left, key, best);
}

int avl_floor(AVL* tree, int key) {
    return floor_rec(tree->root, key, -1);
}

static int ceil_rec(AVLNode* node, int key, int best) {
    if (!node) return best;
    if (node->key == key) return key;
    if (node->key > key) return ceil_rec(node->left, key, node->key);
    return ceil_rec(node->right, key, best);
}

int avl_ceil(AVL* tree, int key) {
    return ceil_rec(tree->root, key, -1);
}

/* Order-statistic tree: select and rank */

static int node_count(AVLNode* n) {
    if (!n) return 0;
    return 1 + node_count(n->left) + node_count(n->right);
}

static int select_rec(AVLNode* node, int k) {
    if (!node) return -1;
    int lc = node_count(node->left);
    if (k <= lc) return select_rec(node->left, k);
    if (k == lc + 1) return node->key;
    return select_rec(node->right, k - lc - 1);
}

int avl_select(AVL* tree, int k) {
    if (k < 1 || k > tree->size) return -1;
    return select_rec(tree->root, k);
}

static int rank_rec(AVLNode* node, int key) {
    if (!node) return 0;
    if (key <= node->key) return rank_rec(node->left, key);
    return 1 + node_count(node->left) + rank_rec(node->right, key);
}

int avl_rank(AVL* tree, int key) {
    return rank_rec(tree->root, key);
}

/* Range queries */

static int count_range(AVLNode* node, int lo, int hi) {
    if (!node) return 0;
    if (node->key < lo) return count_range(node->right, lo, hi);
    if (node->key > hi) return count_range(node->left, lo, hi);
    return 1 + count_range(node->left, lo, hi) + count_range(node->right, lo, hi);
}

int avl_range_count(AVL* tree, int lo, int hi) {
    return count_range(tree->root, lo, hi);
}

static void collect_range(AVLNode* n, int lo, int hi, int* keys, int* vals, int* idx) {
    if (!n) return;
    if (n->key > lo) collect_range(n->left, lo, hi, keys, vals, idx);
    if (n->key >= lo && n->key <= hi) {
        if (keys) keys[*idx] = n->key;
        if (vals) vals[*idx] = n->value;
        (*idx)++;
    }
    if (n->key < hi) collect_range(n->right, lo, hi, keys, vals, idx);
}

int avl_range_query(AVL* tree, int lo, int hi, int* keys, int* values) {
    int idx = 0;
    collect_range(tree->root, lo, hi, keys, values, &idx);
    return idx;
}

/* Traversals */

static void inorder_rec(AVLNode* n, int* keys, int* vals, int* idx) {
    if (!n) return;
    inorder_rec(n->left, keys, vals, idx);
    if (keys) keys[*idx] = n->key;
    if (vals) vals[*idx] = n->value;
    (*idx)++;
    inorder_rec(n->right, keys, vals, idx);
}

void avl_inorder(AVL* tree, int* keys, int* values, int* idx) {
    *idx = 0;
    inorder_rec(tree->root, keys, values, idx);
}

static void preorder_rec(AVLNode* n, int* keys, int* vals, int* idx) {
    if (!n) return;
    if (keys) keys[*idx] = n->key;
    if (vals) vals[*idx] = n->value;
    (*idx)++;
    preorder_rec(n->left, keys, vals, idx);
    preorder_rec(n->right, keys, vals, idx);
}

void avl_preorder(AVL* tree, int* keys, int* values, int* idx) {
    *idx = 0;
    preorder_rec(tree->root, keys, values, idx);
}

static void postorder_rec(AVLNode* n, int* keys, int* vals, int* idx) {
    if (!n) return;
    postorder_rec(n->left, keys, vals, idx);
    postorder_rec(n->right, keys, vals, idx);
    if (keys) keys[*idx] = n->key;
    if (vals) vals[*idx] = n->value;
    (*idx)++;
}

void avl_postorder(AVL* tree, int* keys, int* values, int* idx) {
    *idx = 0;
    postorder_rec(tree->root, keys, values, idx);
}

void avl_level_order(AVL* tree, int* keys, int* values, int* idx) {
    *idx = 0;
    if (!tree->root) return;
    AVLNode* queue[1024];
    int head = 0, tail = 0;
    queue[tail++] = tree->root;
    while (head < tail) {
        AVLNode* n = queue[head++];
        if (keys) keys[*idx] = n->key;
        if (values) values[*idx] = n->value;
        (*idx)++;
        if (n->left) queue[tail++] = n->left;
        if (n->right) queue[tail++] = n->right;
    }
}
