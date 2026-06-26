/**
 * Red-Black Tree Implementation (Guibas & Sedgewick, 1978)
 * L8: Advanced Balanced BST with color invariants
 *
 * Invariants:
 *   1. Every node is RED or BLACK
 *   2. Root is BLACK
 *   3. No two consecutive RED nodes (RED node cannot have RED parent/child)
 *   4. All simple paths from node to leaf have same black-height
 *
 * Theorem: h <= 2*log_2(n+1), all ops O(log n)
 *
 * MIT 6.006 / CLRS Chapter 13
 */

#include "rbtree.h"
#include <stdlib.h>
#include <stdio.h>

static int max_int(int a, int b) { return a > b ? a : b; }

RBTree* rbt_create(void) {
    RBTree* tree = (RBTree*)malloc(sizeof(RBTree));
    tree->nil = (RBNode*)calloc(1, sizeof(RBNode));
    tree->nil->color = RB_BLACK;
    tree->nil->left = tree->nil;
    tree->nil->right = tree->nil;
    tree->nil->parent = tree->nil;
    tree->root = tree->nil;
    tree->size = 0;
    return tree;
}

static RBNode* node_create(RBTree* tree, int key, int value) {
    RBNode* n = (RBNode*)malloc(sizeof(RBNode));
    n->key = key;
    n->value = value;
    n->color = RB_RED;
    n->left = tree->nil;
    n->right = tree->nil;
    n->parent = tree->nil;
    return n;
}

static void free_node_rec(RBTree* tree, RBNode* node) {
    if (node == tree->nil) return;
    free_node_rec(tree, node->left);
    free_node_rec(tree, node->right);
    free(node);
}

void rbt_free(RBTree* tree) {
    free_node_rec(tree, tree->root);
    free(tree->nil);
    free(tree);
}

/* --- Rotations --- */

static void rotate_left(RBTree* tree, RBNode* x) {
    RBNode* y = x->right;
    x->right = y->left;
    if (y->left != tree->nil) y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == tree->nil) tree->root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
}

static void rotate_right(RBTree* tree, RBNode* y) {
    RBNode* x = y->left;
    y->left = x->right;
    if (x->right != tree->nil) x->right->parent = y;
    x->parent = y->parent;
    if (y->parent == tree->nil) tree->root = x;
    else if (y == y->parent->right) y->parent->right = x;
    else y->parent->left = x;
    x->right = y;
    y->parent = x;
}

/* --- Insert Fixup: restore RB properties after insertion --- */

static void insert_fixup(RBTree* tree, RBNode* z) {
    while (z->parent->color == RB_RED) {
        if (z->parent == z->parent->parent->left) {
            RBNode* y = z->parent->parent->right;  /* uncle */
            if (y->color == RB_RED) {
                /* Case 1: uncle is RED */
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    /* Case 2: z is right child */
                    z = z->parent;
                    rotate_left(tree, z);
                }
                /* Case 3: z is left child */
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                rotate_right(tree, z->parent->parent);
            }
        } else {
            /* Symmetric case: parent is right child */
            RBNode* y = z->parent->parent->left;
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rotate_right(tree, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                rotate_left(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = RB_BLACK;
}

void rbt_insert(RBTree* tree, int key, int value) {
    RBNode* z = node_create(tree, key, value);
    RBNode* y = tree->nil;
    RBNode* x = tree->root;
    while (x != tree->nil) {
        y = x;
        if (z->key < x->key) x = x->left;
        else if (z->key > x->key) x = x->right;
        else { /* key exists, update value */
            x->value = value;
            free(z);
            return;
        }
    }
    z->parent = y;
    if (y == tree->nil) tree->root = z;
    else if (z->key < y->key) y->left = z;
    else y->right = z;
    tree->size++;
    insert_fixup(tree, z);
}

/* --- Transplant: replace subtree u with v --- */

static void transplant(RBTree* tree, RBNode* u, RBNode* v) {
    if (u->parent == tree->nil) tree->root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

static RBNode* tree_minimum(RBTree* tree, RBNode* node) {
    while (node->left != tree->nil) node = node->left;
    return node;
}

/* --- Delete Fixup: restore RB properties after deletion --- */

static void delete_fixup(RBTree* tree, RBNode* x) {
    while (x != tree->root && x->color == RB_BLACK) {
        if (x == x->parent->left) {
            RBNode* w = x->parent->right;  /* sibling */
            if (w->color == RB_RED) {
                /* Case 1 */
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                rotate_left(tree, x->parent);
                w = x->parent->right;
            }
            if (w->left->color == RB_BLACK && w->right->color == RB_BLACK) {
                /* Case 2 */
                w->color = RB_RED;
                x = x->parent;
            } else {
                if (w->right->color == RB_BLACK) {
                    /* Case 3 */
                    w->left->color = RB_BLACK;
                    w->color = RB_RED;
                    rotate_right(tree, w);
                    w = x->parent->right;
                }
                /* Case 4 */
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->right->color = RB_BLACK;
                rotate_left(tree, x->parent);
                x = tree->root;
            }
        } else {
            /* Symmetric */
            RBNode* w = x->parent->left;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                rotate_right(tree, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == RB_BLACK && w->left->color == RB_BLACK) {
                w->color = RB_RED;
                x = x->parent;
            } else {
                if (w->left->color == RB_BLACK) {
                    w->right->color = RB_BLACK;
                    w->color = RB_RED;
                    rotate_left(tree, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->left->color = RB_BLACK;
                rotate_right(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = RB_BLACK;
}

void rbt_delete(RBTree* tree, int key) {
    RBNode* z = tree->root;
    while (z != tree->nil) {
        if (key == z->key) break;
        z = (key < z->key) ? z->left : z->right;
    }
    if (z == tree->nil) return;  /* not found */

    RBNode* y = z;
    RBNode* x;
    int y_original_color = y->color;
    if (z->left == tree->nil) {
        x = z->right;
        transplant(tree, z, z->right);
    } else if (z->right == tree->nil) {
        x = z->left;
        transplant(tree, z, z->left);
    } else {
        y = tree_minimum(tree, z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        } else {
            transplant(tree, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        transplant(tree, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }
    free(z);
    tree->size--;
    if (y_original_color == RB_BLACK)
        delete_fixup(tree, x);
}

/* Search */
static RBNode* search_node(RBTree* tree, int key) {
    RBNode* cur = tree->root;
    while (cur != tree->nil) {
        if (key == cur->key) return cur;
        cur = (key < cur->key) ? cur->left : cur->right;
    }
    return tree->nil;
}

int rbt_get(RBTree* tree, int key, int default_val) {
    RBNode* n = search_node(tree, key);
    return (n != tree->nil) ? n->value : default_val;
}

bool rbt_contains(RBTree* tree, int key) {
    return search_node(tree, key) != tree->nil;
}

int rbt_size(RBTree* tree) { return tree->size; }

int rbt_min(RBTree* tree) {
    RBNode* n = tree->root;
    while (n != tree->nil && n->left != tree->nil) n = n->left;
    return (n != tree->nil) ? n->key : -1;
}

int rbt_max(RBTree* tree) {
    RBNode* n = tree->root;
    while (n != tree->nil && n->right != tree->nil) n = n->right;
    return (n != tree->nil) ? n->key : -1;
}

/* Height: longest path from root to leaf */
static int height_rec(RBTree* tree, RBNode* node) {
    if (node == tree->nil) return -1;
    return 1 + max_int(height_rec(tree, node->left), height_rec(tree, node->right));
}

int rbt_height(RBTree* tree) {
    return height_rec(tree, tree->root);
}

/* Black height: number of black nodes on any path root->leaf */
static int bh_rec(RBTree* tree, RBNode* node) {
    if (node == tree->nil) return 1;
    int add = (node->color == RB_BLACK) ? 1 : 0;
    return add + bh_rec(tree, node->left);
}

int rbt_black_height(RBTree* tree) {
    return bh_rec(tree, tree->root);
}

/* Validate Red-Black properties */
static bool validate_rec(RBTree* tree, RBNode* node, int* bh) {
    if (node == tree->nil) { *bh = 1; return true; }
    /* Property 3: no consecutive red */
    if (node->color == RB_RED) {
        if (node->left->color == RB_RED || node->right->color == RB_RED)
            return false;
    }
    int left_bh, right_bh;
    if (!validate_rec(tree, node->left, &left_bh)) return false;
    if (!validate_rec(tree, node->right, &right_bh)) return false;
    if (left_bh != right_bh) return false;
    *bh = left_bh + (node->color == RB_BLACK ? 1 : 0);
    return true;
}

bool rbt_validate(RBTree* tree) {
    if (tree->root->color != RB_BLACK) return false;
    int bh;
    return validate_rec(tree, tree->root, &bh);
}

/* Inorder traversal */
static void inorder_rec(RBTree* tree, RBNode* node, int* keys, int* values, int* idx) {
    if (node == tree->nil) return;
    inorder_rec(tree, node->left, keys, values, idx);
    if (keys) keys[*idx] = node->key;
    if (values) values[*idx] = node->value;
    (*idx)++;
    inorder_rec(tree, node->right, keys, values, idx);
}

void rbt_inorder(RBTree* tree, int* keys, int* values, int* idx) {
    *idx = 0;
    inorder_rec(tree, tree->root, keys, values, idx);
}

/* Dot output */
static void dot_rec(FILE* f, RBTree* tree, RBNode* node, int* id) {
    if (node == tree->nil) return;
    int my_id = (*id)++;
    const char* color = (node->color == RB_RED) ? "red" : "black";
    fprintf(f, "  n%d [label=\"%d\" style=filled fillcolor=%s fontcolor=white];\n",
            my_id, node->key, color);
    if (node->left != tree->nil) {
        int l_id = *id;
        dot_rec(f, tree, node->left, id);
        fprintf(f, "  n%d -> n%d;\n", my_id, l_id);
    }
    if (node->right != tree->nil) {
        int r_id = *id;
        dot_rec(f, tree, node->right, id);
        fprintf(f, "  n%d -> n%d;\n", my_id, r_id);
    }
}

void rbt_print_dot(RBTree* tree, const char* filename) {
    if (tree->root == tree->nil) return;
    FILE* f = fopen(filename, "w");
    if (!f) return;
    fprintf(f, "digraph RBTree {\n");
    int id = 0;
    dot_rec(f, tree, tree->root, &id);
    fprintf(f, "}\n");
    fclose(f);
}
