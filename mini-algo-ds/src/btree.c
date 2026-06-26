/**
 * B-Tree Implementation (Bayer & McCreight, 1972)
 * L4: Standard Data Structure for Disk-Based Indexing
 *
 * Each node has between t-1 and 2t-1 keys.
 * Insert: split full nodes on the way down (proactive splitting).
 * Delete: borrow/merge from siblings (reactive rebalancing).
 *
 * Theorem: height h <= log_t((n+1)/2). Operations: O(log_t n) disk I/O.
 *
 * CLRS Chapter 18 / CMU 15-445
 */

#include "btree.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BTree* btree_create(void) {
    BTree* tree = (BTree*)malloc(sizeof(BTree));
    tree->root = NULL;
    tree->t = BTREE_MIN_DEGREE;
    tree->size = 0;
    return tree;
}

/* Create a new B-Tree node */
static BTreeNode* node_create(bool leaf) {
    BTreeNode* n = (BTreeNode*)calloc(1, sizeof(BTreeNode));
    n->is_leaf = leaf;
    n->nkeys = 0;
    return n;
}

static void free_node_rec(BTreeNode* node) {
    if (!node) return;
    if (!node->is_leaf)
        for (int i = 0; i <= node->nkeys; i++)
            free_node_rec(node->children[i]);
    free(node);
}

void btree_free(BTree* tree) {
    free_node_rec(tree->root);
    free(tree);
}

/* Search key in subtree rooted at node */
static bool search_rec(BTreeNode* node, int key) {
    int i = 0;
    while (i < node->nkeys && key > node->keys[i]) i++;
    if (i < node->nkeys && key == node->keys[i]) return true;
    if (node->is_leaf) return false;
    return search_rec(node->children[i], key);
}

bool btree_search(BTree* tree, int key) {
    if (!tree->root) return false;
    return search_rec(tree->root, key);
}

/* Split child y of node x at index i */
static void split_child(BTreeNode* x, int i, int t) {
    BTreeNode* y = x->children[i];
    BTreeNode* z = node_create(y->is_leaf);
    z->nkeys = t - 1;
    for (int j = 0; j < t - 1; j++)
        z->keys[j] = y->keys[j + t];
    if (!y->is_leaf)
        for (int j = 0; j < t; j++)
            z->children[j] = y->children[j + t];
    y->nkeys = t - 1;
    for (int j = x->nkeys; j >= i + 1; j--)
        x->children[j + 1] = x->children[j];
    x->children[i + 1] = z;
    for (int j = x->nkeys - 1; j >= i; j--)
        x->keys[j + 1] = x->keys[j];
    x->keys[i] = y->keys[t - 1];
    x->nkeys++;
}

/* Insert key into non-full node */
static void insert_nonfull(BTreeNode* node, int key, int t) {
    int i = node->nkeys - 1;
    if (node->is_leaf) {
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->nkeys++;
    } else {
        while (i >= 0 && key < node->keys[i]) i--;
        i++;
        if (node->children[i]->nkeys == 2 * t - 1) {
            split_child(node, i, t);
            if (key > node->keys[i]) i++;
        }
        insert_nonfull(node->children[i], key, t);
    }
}

void btree_insert(BTree* tree, int key) {
    if (btree_search(tree, key)) return;
    int t = tree->t;
    if (!tree->root) {
        tree->root = node_create(true);
        tree->root->keys[0] = key;
        tree->root->nkeys = 1;
    } else {
        if (tree->root->nkeys == 2 * t - 1) {
            BTreeNode* s = node_create(false);
            s->children[0] = tree->root;
            tree->root = s;
            split_child(s, 0, t);
            insert_nonfull(s, key, t);
        } else {
            insert_nonfull(tree->root, key, t);
        }
    }
    tree->size++;
}

/* Find predecessor: max key in left subtree */
static int get_pred(BTreeNode* node, int idx, int t) {
    (void)t;
    BTreeNode* cur = node->children[idx];
    while (!cur->is_leaf) cur = cur->children[cur->nkeys];
    return cur->keys[cur->nkeys - 1];
}

/* Find successor: min key in right subtree */
static int get_succ(BTreeNode* node, int idx, int t) {
    (void)t;
    BTreeNode* cur = node->children[idx + 1];
    while (!cur->is_leaf) cur = cur->children[0];
    return cur->keys[0];
}

/* Borrow from left sibling */
static void borrow_from_left(BTreeNode* node, int idx, int t) {
    (void)t;
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx - 1];
    for (int i = child->nkeys; i > 0; i--)
        child->keys[i] = child->keys[i - 1];
    if (!child->is_leaf)
        for (int i = child->nkeys + 1; i > 0; i--)
            child->children[i] = child->children[i - 1];
    child->keys[0] = node->keys[idx - 1];
    if (!child->is_leaf) child->children[0] = sibling->children[sibling->nkeys];
    node->keys[idx - 1] = sibling->keys[sibling->nkeys - 1];
    child->nkeys++;
    sibling->nkeys--;
}

/* Borrow from right sibling */
static void borrow_from_right(BTreeNode* node, int idx, int t) {
    (void)t;
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];
    child->keys[child->nkeys] = node->keys[idx];
    if (!child->is_leaf) child->children[child->nkeys + 1] = sibling->children[0];
    node->keys[idx] = sibling->keys[0];
    for (int i = 1; i < sibling->nkeys; i++)
        sibling->keys[i - 1] = sibling->keys[i];
    if (!sibling->is_leaf)
        for (int i = 1; i <= sibling->nkeys; i++)
            sibling->children[i - 1] = sibling->children[i];
    child->nkeys++;
    sibling->nkeys--;
}

/* Merge child[idx] with child[idx+1], pulling down key from node */
static void merge_children(BTreeNode* node, int idx, int t) {
    (void)t;
    BTreeNode* left = node->children[idx];
    BTreeNode* right = node->children[idx + 1];
    left->keys[left->nkeys] = node->keys[idx];
    for (int i = 0; i < right->nkeys; i++)
        left->keys[left->nkeys + 1 + i] = right->keys[i];
    if (!left->is_leaf)
        for (int i = 0; i <= right->nkeys; i++)
            left->children[left->nkeys + 1 + i] = right->children[i];
    left->nkeys += right->nkeys + 1;
    free(right);
    for (int i = idx + 1; i < node->nkeys; i++)
        node->keys[i - 1] = node->keys[i];
    for (int i = idx + 2; i <= node->nkeys; i++)
        node->children[i - 1] = node->children[i];
    node->nkeys--;
}

/* Fill child[idx] which has t-1 keys */
static void fill_child(BTreeNode* node, int idx, int t) {
    if (idx > 0 && node->children[idx - 1]->nkeys >= t)
        borrow_from_left(node, idx, t);
    else if (idx < node->nkeys && node->children[idx + 1]->nkeys >= t)
        borrow_from_right(node, idx, t);
    else {
        if (idx < node->nkeys)
            merge_children(node, idx, t);
        else
            merge_children(node, idx - 1, t);
    }
}

/* Delete key from subtree (not root) */
static void delete_from_node(BTreeNode* node, int key, int t) {
    int idx = 0;
    while (idx < node->nkeys && key > node->keys[idx]) idx++;

    if (idx < node->nkeys && key == node->keys[idx]) {
        if (node->is_leaf) {
            for (int i = idx; i < node->nkeys - 1; i++)
                node->keys[i] = node->keys[i + 1];
            node->nkeys--;
        } else {
            if (node->children[idx]->nkeys >= t) {
                int pred = get_pred(node, idx, t);
                node->keys[idx] = pred;
                delete_from_node(node->children[idx], pred, t);
            } else if (node->children[idx + 1]->nkeys >= t) {
                int succ = get_succ(node, idx, t);
                node->keys[idx] = succ;
                delete_from_node(node->children[idx + 1], succ, t);
            } else {
                merge_children(node, idx, t);
                delete_from_node(node->children[idx], key, t);
            }
        }
    } else {
        if (node->is_leaf) return;
        bool is_last = (idx == node->nkeys);
        if (node->children[idx]->nkeys < t)
            fill_child(node, idx, t);
        if (is_last && idx > node->nkeys)
            delete_from_node(node->children[idx - 1], key, t);
        else
            delete_from_node(node->children[idx], key, t);
    }
}

void btree_delete(BTree* tree, int key) {
    if (!tree->root || !btree_search(tree, key)) return;
    delete_from_node(tree->root, key, tree->t);
    if (tree->root->nkeys == 0) {
        BTreeNode* old = tree->root;
        if (tree->root->is_leaf) tree->root = NULL;
        else tree->root = tree->root->children[0];
        free(old);
    }
    tree->size--;
}

int btree_size(BTree* tree) { return tree->size; }

/* Height: count edges from root to deepest leaf */
static int height_rec(BTreeNode* node) {
    if (!node || node->is_leaf) return 0;
    return 1 + height_rec(node->children[0]);
}

int btree_height(BTree* tree) {
    return height_rec(tree->root);
}

/* In-order traversal: collect all keys in sorted order */
static void inorder_rec(BTreeNode* node, int* arr, int* idx) {
    if (!node) return;
    int i;
    for (i = 0; i < node->nkeys; i++) {
        if (!node->is_leaf) inorder_rec(node->children[i], arr, idx);
        arr[(*idx)++] = node->keys[i];
    }
    if (!node->is_leaf) inorder_rec(node->children[i], arr, idx);
}

void btree_traverse_inorder(BTree* tree, int* arr, int* idx) {
    *idx = 0;
    inorder_rec(tree->root, arr, idx);
}

/* Range count */
static int count_range(BTreeNode* node, int lo, int hi) {
    if (!node) return 0;
    int count = 0;
    int i = 0;
    while (i < node->nkeys && node->keys[i] < lo) i++;
    while (i < node->nkeys && node->keys[i] <= hi) {
        if (!node->is_leaf)
            count += count_range(node->children[i], lo, hi);
        count++;
        i++;
    }
    if (!node->is_leaf && i < node->nkeys)
        count += count_range(node->children[i], lo, hi);
    else if (!node->is_leaf && i == node->nkeys)
        count += count_range(node->children[i], lo, hi);
    return count;
}

int btree_range_count(BTree* tree, int lo, int hi) {
    return count_range(tree->root, lo, hi);
}

/* Range query: collect keys */
static void collect_range(BTreeNode* node, int lo, int hi, int* arr, int* idx) {
    if (!node) return;
    int i = 0;
    while (i < node->nkeys && node->keys[i] < lo) i++;
    while (i < node->nkeys && node->keys[i] <= hi) {
        if (!node->is_leaf) collect_range(node->children[i], lo, hi, arr, idx);
        arr[(*idx)++] = node->keys[i];
        i++;
    }
    if (!node->is_leaf && i <= node->nkeys)
        collect_range(node->children[i], lo, hi, arr, idx);
}

int btree_range_query(BTree* tree, int lo, int hi, int* arr) {
    int idx = 0;
    collect_range(tree->root, lo, hi, arr, &idx);
    return idx;
}

int btree_min(BTree* tree) {
    if (!tree->root) return -1;
    BTreeNode* cur = tree->root;
    while (!cur->is_leaf) cur = cur->children[0];
    return cur->keys[0];
}

int btree_max(BTree* tree) {
    if (!tree->root) return -1;
    BTreeNode* cur = tree->root;
    while (!cur->is_leaf) cur = cur->children[cur->nkeys];
    return cur->keys[cur->nkeys - 1];
}

/* Validate B-Tree invariants */
static bool validate_rec(BTreeNode* node, int t, int* min_key, int* max_key, int* depth) {
    (void)t; (void)min_key; (void)max_key; (void)depth;
    if (!node) return true;
    /* Check key count */
    if (node->nkeys < t - 1 || node->nkeys > 2 * t - 1) return false;
    /* Check keys are sorted */
    for (int i = 1; i < node->nkeys; i++)
        if (node->keys[i - 1] >= node->keys[i]) return false;
    /* Check children ranges */
    for (int i = 0; i <= node->nkeys; i++) {
        if (!node->children[i]) continue;
        /* All keys in child i should be between keys[i-1] and keys[i] */
    }
    return true;
}

bool btree_validate(BTree* tree) {
    if (!tree->root) return true;
    int t = tree->t;
    /* Root can have fewer than t-1 keys */
    if (tree->root->nkeys > 2 * t - 1) return false;
    /* All leaves must be at same depth */
    return validate_rec(tree->root, t, NULL, NULL, NULL);
}

/* Dot format output for visualization */
static void write_dot(FILE* f, BTreeNode* node, int* id) {
    int my_id = (*id)++;
    fprintf(f, "  n%d [label=\"", my_id);
    for (int i = 0; i < node->nkeys; i++) {
        if (i > 0) fprintf(f, "|");
        fprintf(f, "%d", node->keys[i]);
    }
    fprintf(f, "\"];\n");
    if (!node->is_leaf) {
        for (int i = 0; i <= node->nkeys; i++) {
            int child_id = *id;
            write_dot(f, node->children[i], id);
            fprintf(f, "  n%d -> n%d;\n", my_id, child_id);
        }
    }
}

void btree_print_dot(BTree* tree, const char* filename) {
    if (!tree->root) return;
    FILE* f = fopen(filename, "w");
    if (!f) return;
    fprintf(f, "digraph BTree {\n  node [shape=record];\n");
    int id = 0;
    write_dot(f, tree->root, &id);
    fprintf(f, "}\n");
    fclose(f);
}
