#include "tree.h"
#include "combo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TreeNode* tree_create_node(int key) {
    TreeNode* node = malloc(sizeof(TreeNode));
    node->key = key;
    node->height = 1;
    node->left = node->right = NULL;
    return node;
}

void tree_free(TreeNode* root) {
    if (!root) return;
    tree_free(root->left);
    tree_free(root->right);
    free(root);
}

TreeNode* bst_insert(TreeNode* root, int key) {
    if (!root) return tree_create_node(key);
    if (key < root->key)
        root->left = bst_insert(root->left, key);
    else if (key > root->key)
        root->right = bst_insert(root->right, key);
    return root;
}

TreeNode* bst_search(TreeNode* root, int key) {
    if (!root || root->key == key) return root;
    if (key < root->key) return bst_search(root->left, key);
    return bst_search(root->right, key);
}

TreeNode* bst_min(TreeNode* root) {
    while (root && root->left) root = root->left;
    return root;
}

TreeNode* bst_max(TreeNode* root) {
    while (root && root->right) root = root->right;
    return root;
}

TreeNode* bst_delete(TreeNode* root, int key) {
    if (!root) return NULL;
    if (key < root->key)
        root->left = bst_delete(root->left, key);
    else if (key > root->key)
        root->right = bst_delete(root->right, key);
    else {
        if (!root->left) {
            TreeNode* temp = root->right;
            free(root);
            return temp;
        }
        if (!root->right) {
            TreeNode* temp = root->left;
            free(root);
            return temp;
        }
        TreeNode* temp = bst_min(root->right);
        root->key = temp->key;
        root->right = bst_delete(root->right, temp->key);
    }
    return root;
}

void tree_preorder(TreeNode* root, int* arr, int* idx) {
    if (!root) return;
    arr[(*idx)++] = root->key;
    tree_preorder(root->left, arr, idx);
    tree_preorder(root->right, arr, idx);
}

void tree_inorder(TreeNode* root, int* arr, int* idx) {
    if (!root) return;
    tree_inorder(root->left, arr, idx);
    arr[(*idx)++] = root->key;
    tree_inorder(root->right, arr, idx);
}

void tree_postorder(TreeNode* root, int* arr, int* idx) {
    if (!root) return;
    tree_postorder(root->left, arr, idx);
    tree_postorder(root->right, arr, idx);
    arr[(*idx)++] = root->key;
}

void tree_levelorder(TreeNode* root, int* arr, int* idx) {
    if (!root) return;
    TreeNode* queue[1024];
    int head = 0, tail = 0;
    queue[tail++] = root;
    while (head < tail) {
        TreeNode* node = queue[head++];
        arr[(*idx)++] = node->key;
        if (node->left) queue[tail++] = node->left;
        if (node->right) queue[tail++] = node->right;
    }
}

int tree_height(TreeNode* root) {
    if (!root) return 0;
    int lh = tree_height(root->left);
    int rh = tree_height(root->right);
    return 1 + (lh > rh ? lh : rh);
}

int tree_size(TreeNode* root) {
    if (!root) return 0;
    return 1 + tree_size(root->left) + tree_size(root->right);
}

bool tree_is_bst(TreeNode* root, int min, int max) {
    if (!root) return true;
    if (root->key < min || root->key > max) return false;
    return tree_is_bst(root->left, min, root->key) &&
           tree_is_bst(root->right, root->key, max);
}

void tree_print(TreeNode* root, int space) {
    if (!root) return;
    space += 3;
    tree_print(root->right, space);
    printf("\n");
    for (int i = 3; i < space; i++) printf(" ");
    printf("%d\n", root->key);
    tree_print(root->left, space);
}

static int height(TreeNode* n) { return n ? n->height : 0; }
static int max(int a, int b) { return a > b ? a : b; }
static int get_balance(TreeNode* n) {
    return n ? height(n->left) - height(n->right) : 0;
}

TreeNode* avl_rotate_right(TreeNode* y) {
    TreeNode* x = y->left;
    TreeNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;
    return x;
}

TreeNode* avl_rotate_left(TreeNode* x) {
    TreeNode* y = x->right;
    TreeNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;
    return y;
}

TreeNode* avl_insert(TreeNode* root, int key) {
    if (!root) return tree_create_node(key);

    if (key < root->key)
        root->left = avl_insert(root->left, key);
    else if (key > root->key)
        root->right = avl_insert(root->right, key);
    else
        return root;

    root->height = 1 + max(height(root->left), height(root->right));
    int balance = get_balance(root);

    // LL
    if (balance > 1 && key < root->left->key)
        return avl_rotate_right(root);
    // RR
    if (balance < -1 && key > root->right->key)
        return avl_rotate_left(root);
    // LR
    if (balance > 1 && key > root->left->key) {
        root->left = avl_rotate_left(root->left);
        return avl_rotate_right(root);
    }
    // RL
    if (balance < -1 && key < root->right->key) {
        root->right = avl_rotate_right(root->right);
        return avl_rotate_left(root);
    }
    return root;
}

HuffmanNode* huffman_build_tree(char* text, int freq[256]) {
    memset(freq, 0, 256 * sizeof(int));
    for (int i = 0; text[i]; i++)
        freq[(unsigned char)text[i]]++;

    int n = 0;
    HuffmanNode* nodes[256];

    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            HuffmanNode* node = malloc(sizeof(HuffmanNode));
            node->ch = (char)i;
            node->freq = freq[i];
            node->left = node->right = NULL;
            nodes[n++] = node;
        }
    }

    int size = n;
    while (size > 1) {
        int min1 = 0, min2 = 1;
        if (nodes[min1]->freq > nodes[min2]->freq) {
            int t = min1; min1 = min2; min2 = t;
        }
        for (int i = 2; i < size; i++) {
            if (nodes[i]->freq < nodes[min1]->freq) {
                min2 = min1; min1 = i;
            } else if (nodes[i]->freq < nodes[min2]->freq) {
                min2 = i;
            }
        }

        HuffmanNode* internal = malloc(sizeof(HuffmanNode));
        internal->ch = '\0';
        internal->freq = nodes[min1]->freq + nodes[min2]->freq;
        internal->left = nodes[min1];
        internal->right = nodes[min2];

        nodes[min1] = internal;
        nodes[min2] = nodes[size - 1];
        size--;
    }

    return nodes[0];
}

void huffman_generate_codes(HuffmanNode* root, char* code, int len, HuffmanCode codes[256]) {
    if (!root) return;
    if (!root->left && !root->right) {
        codes[(unsigned char)root->ch].ch = root->ch;
        memcpy(codes[(unsigned char)root->ch].code, code, len);
        codes[(unsigned char)root->ch].code_len = len;
        return;
    }
    code[len] = '0';
    huffman_generate_codes(root->left, code, len + 1, codes);
    code[len] = '1';
    huffman_generate_codes(root->right, code, len + 1, codes);
}

char* huffman_encode(char* text, HuffmanCode codes[256]) {
    int len = 0;
    for (int i = 0; text[i]; i++)
        len += codes[(unsigned char)text[i]].code_len;
    char* encoded = malloc(len + 1);
    int pos = 0;
    for (int i = 0; text[i]; i++) {
        HuffmanCode hc = codes[(unsigned char)text[i]];
        for (int j = 0; j < hc.code_len; j++)
            encoded[pos++] = hc.code[j];
    }
    encoded[pos] = '\0';
    return encoded;
}

void huffman_free(HuffmanNode* root) {
    if (!root) return;
    huffman_free(root->left);
    huffman_free(root->right);
    free(root);
}

char* huffman_decode(char* encoded, HuffmanNode* root) {
    int maxlen = strlen(encoded) + 1;
    char* decoded = malloc(maxlen);
    int pos = 0;
    HuffmanNode* curr = root;
    for (int i = 0; encoded[i]; i++) {
        curr = (encoded[i] == '0') ? curr->left : curr->right;
        if (!curr->left && !curr->right) {
            decoded[pos++] = curr->ch;
            curr = root;
        }
    }
    decoded[pos] = '\0';
    return decoded;
}

/* ===== Red-Black Tree =====
 * Properties:
 * 1. Every node is RED or BLACK.
 * 2. Root is BLACK.
 * 3. Leaves (NULL) are BLACK.
 * 4. RED node's children are BLACK (no two consecutive REDs).
 * 5. Every path from root to leaf has same # BLACK nodes (black-height).
 * Insertion: O(log n). Reference: CLRS §13, Guibas-Sedgewick (1978) */
#define RB_BLACK 0
#define RB_RED   1

RBNode* rb_create_node(int key) {
    RBNode* n = malloc(sizeof(RBNode));
    n->key = key;
    n->color = RB_RED;  /* new nodes are RED */
    n->left = n->right = n->parent = NULL;
    return n;
}

static RBNode* rb_rotate_left(RBNode* root, RBNode* x) {
    RBNode* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
    return root;
}

static RBNode* rb_rotate_right(RBNode* root, RBNode* x) {
    RBNode* y = x->left;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    y->parent = x->parent;
    if (!x->parent) root = y;
    else if (x == x->parent->right) x->parent->right = y;
    else x->parent->left = y;
    y->right = x;
    x->parent = y;
    return root;
}

static RBNode* rb_fixup(RBNode* root, RBNode* z) {
    while (z->parent && z->parent->color == RB_RED) {
        if (z->parent == z->parent->parent->left) {
            RBNode* y = z->parent->parent->right;
            if (y && y->color == RB_RED) {
                /* Case 1: uncle is RED */
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    /* Case 2: z is right child */
                    z = z->parent;
                    root = rb_rotate_left(root, z);
                }
                /* Case 3: z is left child */
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                root = rb_rotate_right(root, z->parent->parent);
            }
        } else {
            RBNode* y = z->parent->parent->left;
            if (y && y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    root = rb_rotate_right(root, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                root = rb_rotate_left(root, z->parent->parent);
            }
        }
    }
    root->color = RB_BLACK;
    return root;
}

RBNode* rb_insert(RBNode* root, int key) {
    RBNode* z = rb_create_node(key);
    RBNode* y = NULL;
    RBNode* x = root;
    while (x) {
        y = x;
        if (z->key < x->key) x = x->left;
        else x = x->right;
    }
    z->parent = y;
    if (!y) root = z;
    else if (z->key < y->key) y->left = z;
    else y->right = z;
    root = rb_fixup(root, z);
    return root;
}

RBNode* rb_search(RBNode* root, int key) {
    while (root && root->key != key) {
        if (key < root->key) root = root->left;
        else root = root->right;
    }
    return root;
}

void rb_free(RBNode* root) {
    if (!root) return;
    rb_free(root->left);
    rb_free(root->right);
    free(root);
}

/* ===== Segment Tree =====
 * Range sum queries and point updates in O(log n).
 * Build: O(n). Query/Update: O(log n).
 * Reference: Bentley (1977), MIT 6.851 */
SegTree* segtree_create(long long arr[], int n) {
    SegTree* st = malloc(sizeof(SegTree));
    st->n = n;
    st->tree = calloc(4 * n, sizeof(long long));
    /* build from leaves */
    for (int i = 0; i < n; i++)
        st->tree[n + i] = arr[i];
    for (int i = n - 1; i > 0; i--)
        st->tree[i] = st->tree[2 * i] + st->tree[2 * i + 1];
    return st;
}

static void st_update_internal(SegTree* st, int p, long long val) {
    int n = st->n;
    p += n;
    st->tree[p] = val;
    for (p /= 2; p > 0; p /= 2)
        st->tree[p] = st->tree[2 * p] + st->tree[2 * p + 1];
}

void segtree_update(SegTree* st, int idx, long long val) {
    st_update_internal(st, idx, val);
}

static long long st_query_internal(SegTree* st, int l, int r) {
    int n = st->n;
    long long res = 0;
    for (l += n, r += n; l < r; l /= 2, r /= 2) {
        if (l & 1) res += st->tree[l++];
        if (r & 1) res += st->tree[--r];
    }
    return res;
}

long long segtree_query(SegTree* st, int ql, int qr) {
    return st_query_internal(st, ql, qr + 1);
}

void segtree_free(SegTree* st) {
    free(st->tree);
    free(st);
}

/* ===== Fenwick Tree (Binary Indexed Tree) =====
 * Point add O(log n), prefix sum O(log n).
 * Uses bit[i] = sum of range (i - lsb(i), i].
 * lsb(i) = i & -i.
 * Reference: Fenwick (1994), CLRS §Problems 14 */
FenwickTree* fenwick_create(int n) {
    FenwickTree* ft = malloc(sizeof(FenwickTree));
    ft->n = n;
    ft->bit = calloc(n + 1, sizeof(long long));
    return ft;
}

void fenwick_add(FenwickTree* ft, int idx, long long delta) {
    for (int i = idx + 1; i <= ft->n; i += i & -i)
        ft->bit[i] += delta;
}

long long fenwick_sum(FenwickTree* ft, int idx) {
    long long s = 0;
    for (int i = idx + 1; i > 0; i -= i & -i)
        s += ft->bit[i];
    return s;
}

long long fenwick_range(FenwickTree* ft, int l, int r) {
    return fenwick_sum(ft, r) - fenwick_sum(ft, l - 1);
}

void fenwick_free(FenwickTree* ft) {
    free(ft->bit);
    free(ft);
}

/* ===== Binary Min-Heap =====
 * insert: O(log n). extract-min: O(log n). peek: O(1).
 * Reference: Williams (1964), CLRS §6 */
MinHeap* heap_create(int cap) {
    MinHeap* h = malloc(sizeof(MinHeap));
    h->heap = malloc(cap * sizeof(int));
    h->size = 0;
    h->cap = cap;
    return h;
}

static void heap_swap(int* a, int* b) { int t = *a; *a = *b; *b = t; }

static void heap_sift_up(MinHeap* h, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (h->heap[parent] <= h->heap[idx]) break;
        heap_swap(&h->heap[parent], &h->heap[idx]);
        idx = parent;
    }
}

static void heap_sift_down(MinHeap* h, int idx) {
    while (1) {
        int smallest = idx;
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        if (left < h->size && h->heap[left] < h->heap[smallest])
            smallest = left;
        if (right < h->size && h->heap[right] < h->heap[smallest])
            smallest = right;
        if (smallest == idx) break;
        heap_swap(&h->heap[idx], &h->heap[smallest]);
        idx = smallest;
    }
}

void heap_insert(MinHeap* h, int val) {
    if (h->size >= h->cap) return;
    h->heap[h->size] = val;
    heap_sift_up(h, h->size);
    h->size++;
}

int heap_extract_min(MinHeap* h) {
    if (h->size == 0) return -1;
    int min = h->heap[0];
    h->heap[0] = h->heap[--h->size];
    heap_sift_down(h, 0);
    return min;
}

int heap_peek(MinHeap* h) {
    return h->size > 0 ? h->heap[0] : -1;
}

void heap_free(MinHeap* h) {
    free(h->heap);
    free(h);
}

/* ===== Trie (Prefix Tree) =====
 * Insert and search in O(L) where L = word length.
 * Reference: Fredkin (1960), Knuth Vol.3 §6.3 */
TrieNode* trie_create(void) {
    TrieNode* node = malloc(sizeof(TrieNode));
    for (int i = 0; i < TRIE_ALPHABET; i++)
        node->children[i] = NULL;
    node->is_end = false;
    return node;
}

void trie_insert(TrieNode* root, const char* word) {
    TrieNode* curr = root;
    for (int i = 0; word[i]; i++) {
        int idx = word[i] - 'a';
        if (idx < 0 || idx >= TRIE_ALPHABET) continue;
        if (!curr->children[idx])
            curr->children[idx] = trie_create();
        curr = curr->children[idx];
    }
    curr->is_end = true;
}

bool trie_search(TrieNode* root, const char* word) {
    TrieNode* curr = root;
    for (int i = 0; word[i]; i++) {
        int idx = word[i] - 'a';
        if (idx < 0 || idx >= TRIE_ALPHABET) return false;
        if (!curr->children[idx]) return false;
        curr = curr->children[idx];
    }
    return curr->is_end;
}

bool trie_starts_with(TrieNode* root, const char* prefix) {
    TrieNode* curr = root;
    for (int i = 0; prefix[i]; i++) {
        int idx = prefix[i] - 'a';
        if (idx < 0 || idx >= TRIE_ALPHABET) return false;
        if (!curr->children[idx]) return false;
        curr = curr->children[idx];
    }
    return true;
}

void trie_free(TrieNode* root) {
    if (!root) return;
    for (int i = 0; i < TRIE_ALPHABET; i++)
        trie_free(root->children[i]);
    free(root);
}

/* ===== Lowest Common Ancestor (Binary Lifting) =====
 * Preprocess O(N log N), query O(log N).
 * up[v][j] = 2^j-th ancestor of v.
 * Reference: Bender-Farach-Colton (2000), MIT 6.851 */
LCA* lca_build(int n, int parent[], int root) {
    LCA* lca = malloc(sizeof(LCA));
    lca->n = n;
    lca->log_n = 0;
    while ((1 << lca->log_n) <= n) lca->log_n++;
    if (lca->log_n < 1) lca->log_n = 1;

    lca->up = malloc(n * sizeof(int*));
    lca->depth = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        lca->up[i] = calloc(lca->log_n, sizeof(int));
        lca->depth[i] = 0;
    }

    /* DFS to set depths and parents */
    int stack[512], top = 0;
    bool visited[512] = {false};
    stack[top++] = root;
    lca->depth[root] = 0;
    for (int i = 0; i < n; i++) lca->up[i][0] = parent[i];

    while (top > 0) {
        int u = stack[--top];
        if (!visited[u]) {
            visited[u] = true;
            for (int v = 0; v < n; v++) {
                if (parent[v] == u && !visited[v]) {
                    lca->depth[v] = lca->depth[u] + 1;
                    stack[top++] = v;
                }
            }
        }
    }

    /* fill binary lifting table */
    for (int j = 1; j < lca->log_n; j++)
        for (int i = 0; i < n; i++)
            if (lca->up[i][j - 1] != -1)
                lca->up[i][j] = lca->up[lca->up[i][j - 1]][j - 1];
            else
                lca->up[i][j] = -1;

    return lca;
}

int lca_query(LCA* lca, int u, int v) {
    if (lca->depth[u] < lca->depth[v]) { int t = u; u = v; v = t; }
    /* lift u to depth of v */
    int diff = lca->depth[u] - lca->depth[v];
    for (int j = 0; j < lca->log_n; j++)
        if (diff & (1 << j))
            u = lca->up[u][j];
    if (u == v) return u;
    for (int j = lca->log_n - 1; j >= 0; j--)
        if (lca->up[u][j] != lca->up[v][j]) {
            u = lca->up[u][j];
            v = lca->up[v][j];
        }
    return lca->up[u][0];
}

void lca_free(LCA* lca) {
    for (int i = 0; i < lca->n; i++) free(lca->up[i]);
    free(lca->up);
    free(lca->depth);
    free(lca);
}

/* Tree diameter: longest path in tree.
 * Method: 2 BFS/DFS passes. 1st pass: farthest from any node.
 * 2nd pass: farthest from that node → diameter.
 * Reference: MIT 6.042J Tree Theory */
int tree_diameter_from_adj(int n, int adj[][MAX_VERTICES], int deg[]) {
    if (n <= 1) return 0;
    int dist[MAX_VERTICES];
    bool visited[MAX_VERTICES];

    /* BFS from node 0 */
    int queue[MAX_VERTICES], head, tail;
    int farthest = 0, max_dist = 0;

    for (int pass = 0; pass < 2; pass++) {
        memset(visited, 0, sizeof(visited));
        head = tail = 0;
        int start = (pass == 0) ? 0 : farthest;
        queue[tail++] = start;
        visited[start] = true;
        dist[start] = 0;
        max_dist = 0;

        while (head < tail) {
            int u = queue[head++];
            if (dist[u] > max_dist) {
                max_dist = dist[u];
                farthest = u;
            }
            for (int i = 0; i < deg[u]; i++) {
                int v = adj[u][i];
                if (!visited[v]) {
                    visited[v] = true;
                    dist[v] = dist[u] + 1;
                    queue[tail++] = v;
                }
            }
        }
    }
    return max_dist;
}

/* Inorder successor in BST
 * If right child exists: min of right subtree.
 * Otherwise: walk up to first ancestor where node is in left subtree.
 * Reference: CLRS §12.2 */
TreeNode* bst_successor(TreeNode* root, TreeNode* node) {
    (void)root;
    if (node->right) return bst_min(node->right);
    /* Walk up using parent links — simplified: search from root */
    TreeNode* succ = NULL;
    TreeNode* curr = root;
    while (curr) {
        if (node->key < curr->key) {
            succ = curr;
            curr = curr->left;
        } else if (node->key > curr->key) {
            curr = curr->right;
        } else {
            break;
        }
    }
    return succ;
}

/* Inorder predecessor: max of left subtree, or walk up */
TreeNode* bst_predecessor(TreeNode* root, TreeNode* node) {
    (void)root;
    if (node->left) return bst_max(node->left);
    TreeNode* pred = NULL;
    TreeNode* curr = root;
    while (curr) {
        if (node->key > curr->key) {
            pred = curr;
            curr = curr->right;
        } else if (node->key < curr->key) {
            curr = curr->left;
        } else {
            break;
        }
    }
    return pred;
}

/* Check if tree is balanced: |left.height - right.height| ≤ 1 for all nodes.
 * Returns height if balanced, -1 if not (O(n) single pass).
 * Reference: CLRS §Problems 13 */
static int balance_check(TreeNode* root) {
    if (!root) return 0;
    int lh = balance_check(root->left);
    if (lh == -1) return -1;
    int rh = balance_check(root->right);
    if (rh == -1) return -1;
    if (abs(lh - rh) > 1) return -1;
    return 1 + (lh > rh ? lh : rh);
}

bool tree_is_balanced(TreeNode* root) {
    return balance_check(root) != -1;
}

/* Count BSTs with n nodes (Catalan number C_n)
 * Recurrence: T(n) = Σ T(i)*T(n-1-i) for i=0..n-1, T(0)=1.
 * This is the n-th Catalan number.
 * Reference: MIT 6.042J, Stanley */
long long count_bsts(int n) {
    long long catalan = combination(2 * n, n) / (n + 1);
    return catalan;
}
