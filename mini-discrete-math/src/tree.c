#include "tree.h"
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
