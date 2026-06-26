#ifndef TREE_H
#define TREE_H

#include <stdbool.h>

#define MAX_VERTICES 64

typedef struct TreeNode {
    int key;
    int height;          // for AVL
    struct TreeNode *left, *right;
} TreeNode;

TreeNode* tree_create_node(int key);
void tree_free(TreeNode* root);

TreeNode* bst_insert(TreeNode* root, int key);
TreeNode* bst_search(TreeNode* root, int key);
TreeNode* bst_delete(TreeNode* root, int key);
TreeNode* bst_min(TreeNode* root);
TreeNode* bst_max(TreeNode* root);

void tree_preorder(TreeNode* root, int* arr, int* idx);
void tree_inorder(TreeNode* root, int* arr, int* idx);
void tree_postorder(TreeNode* root, int* arr, int* idx);
void tree_levelorder(TreeNode* root, int* arr, int* idx);

int tree_height(TreeNode* root);
int tree_size(TreeNode* root);
bool tree_is_bst(TreeNode* root, int min, int max);
void tree_print(TreeNode* root, int space);

TreeNode* avl_insert(TreeNode* root, int key);
TreeNode* avl_rotate_right(TreeNode* y);
TreeNode* avl_rotate_left(TreeNode* x);

typedef struct HuffmanNode {
    char ch;
    int freq;
    struct HuffmanNode *left, *right;
} HuffmanNode;

typedef struct {
    char ch;
    char code[256];
    int code_len;
} HuffmanCode;

HuffmanNode* huffman_build_tree(char* text, int freq[256]);
void huffman_generate_codes(HuffmanNode* root, char* code, int len, HuffmanCode codes[256]);
char* huffman_encode(char* text, HuffmanCode codes[256]);
char* huffman_decode(char* encoded, HuffmanNode* root);
void huffman_free(HuffmanNode* root);

/* Red-Black Tree node (color as int: 0=BLACK, 1=RED) */
typedef struct RBNode {
    int key;
    int color; /* 0=BLACK, 1=RED */
    struct RBNode *left, *right, *parent;
} RBNode;

RBNode* rb_create_node(int key);
RBNode* rb_insert(RBNode* root, int key);
RBNode* rb_search(RBNode* root, int key);
void rb_free(RBNode* root);

/* Segment Tree: range sum queries and point updates, O(log n) */
typedef struct {
    long long* tree;
    int n;
} SegTree;

SegTree* segtree_create(long long arr[], int n);
void segtree_update(SegTree* st, int idx, long long val);
long long segtree_query(SegTree* st, int ql, int qr);
void segtree_free(SegTree* st);

/* Fenwick Tree (Binary Indexed Tree): prefix sums, O(log n) */
typedef struct {
    long long* bit;
    int n;
} FenwickTree;

FenwickTree* fenwick_create(int n);
void fenwick_add(FenwickTree* ft, int idx, long long delta);
long long fenwick_sum(FenwickTree* ft, int idx);
long long fenwick_range(FenwickTree* ft, int l, int r);
void fenwick_free(FenwickTree* ft);

/* Binary Heap (min-heap) */
typedef struct {
    int* heap;
    int size;
    int cap;
} MinHeap;

MinHeap* heap_create(int cap);
void heap_insert(MinHeap* h, int val);
int heap_extract_min(MinHeap* h);
int heap_peek(MinHeap* h);
void heap_free(MinHeap* h);

/* Trie (prefix tree) for lowercase a-z */
#define TRIE_ALPHABET 26
typedef struct TrieNode {
    struct TrieNode* children[TRIE_ALPHABET];
    bool is_end;
} TrieNode;

TrieNode* trie_create(void);
void trie_insert(TrieNode* root, const char* word);
bool trie_search(TrieNode* root, const char* word);
bool trie_starts_with(TrieNode* root, const char* prefix);
void trie_free(TrieNode* root);

/* Lowest Common Ancestor (binary lifting, O(N log N) prep, O(log N) query) */
typedef struct {
    int** up;
    int* depth;
    int n;
    int log_n;
} LCA;

LCA* lca_build(int n, int parent[], int root);
int lca_query(LCA* lca, int u, int v);
void lca_free(LCA* lca);

/* Tree diameter: longest path in tree (2-DFS method) */
int tree_diameter_from_adj(int n, int adj[][MAX_VERTICES], int deg[]);

/* Inorder successor in BST (Morris traversal-based) */
TreeNode* bst_successor(TreeNode* root, TreeNode* node);
TreeNode* bst_predecessor(TreeNode* root, TreeNode* node);

/* Check if tree is balanced (|left.height - right.height| ≤ 1) */
bool tree_is_balanced(TreeNode* root);

/* Count BSTs with n nodes (Catalan number) — structural combinatorics */
long long count_bsts(int n);

#endif
