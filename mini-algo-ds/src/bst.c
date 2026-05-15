#include "bst.h"
#include <stdlib.h>
#include <stdio.h>

static BSTNode* bst_node_create(int key) {
    BSTNode* node = (BSTNode*)malloc(sizeof(BSTNode));
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    return node;
}

BST* bst_create(void) {
    BST* tree = (BST*)malloc(sizeof(BST));
    tree->root = NULL;
    tree->size = 0;
    return tree;
}

static void bst_free_nodes(BSTNode* node) {
    if (node == NULL) return;
    bst_free_nodes(node->left);
    bst_free_nodes(node->right);
    free(node);
}

void bst_free(BST* tree) {
    bst_free_nodes(tree->root);
    free(tree);
}

static BSTNode* bst_insert_node(BSTNode* node, int key) {
    if (node == NULL) return bst_node_create(key);
    if (key < node->key) {
        node->left = bst_insert_node(node->left, key);
    } else if (key > node->key) {
        node->right = bst_insert_node(node->right, key);
    }
    return node;
}

void bst_insert(BST* tree, int key) {
    if (!bst_search(tree, key)) {
        tree->root = bst_insert_node(tree->root, key);
        tree->size++;
    }
}

static bool bst_search_node(BSTNode* node, int key) {
    if (node == NULL) return false;
    if (key == node->key) return true;
    if (key < node->key) return bst_search_node(node->left, key);
    return bst_search_node(node->right, key);
}

bool bst_search(BST* tree, int key) {
    return bst_search_node(tree->root, key);
}

BSTNode* bst_min(BSTNode* node) {
    while (node && node->left != NULL) {
        node = node->left;
    }
    return node;
}

BSTNode* bst_max(BSTNode* node) {
    while (node && node->right != NULL) {
        node = node->right;
    }
    return node;
}

static BSTNode* bst_delete_node(BSTNode* node, int key) {
    if (node == NULL) return NULL;
    if (key < node->key) {
        node->left = bst_delete_node(node->left, key);
    } else if (key > node->key) {
        node->right = bst_delete_node(node->right, key);
    } else {
        if (node->left == NULL) {
            BSTNode* temp = node->right;
            free(node);
            return temp;
        }
        if (node->right == NULL) {
            BSTNode* temp = node->left;
            free(node);
            return temp;
        }
        BSTNode* successor = bst_min(node->right);
        node->key = successor->key;
        node->right = bst_delete_node(node->right, successor->key);
    }
    return node;
}

void bst_delete(BST* tree, int key) {
    if (bst_search(tree, key)) {
        tree->root = bst_delete_node(tree->root, key);
        tree->size--;
    }
}

static int bst_height_node(BSTNode* node) {
    if (node == NULL) return -1;
    int lh = bst_height_node(node->left);
    int rh = bst_height_node(node->right);
    return 1 + (lh > rh ? lh : rh);
}

int bst_height(BST* tree) {
    return bst_height_node(tree->root);
}

int bst_size(BST* tree) {
    return tree->size;
}

static void inorder_traverse(BSTNode* node, int* arr, int* idx) {
    if (node == NULL) return;
    inorder_traverse(node->left, arr, idx);
    arr[(*idx)++] = node->key;
    inorder_traverse(node->right, arr, idx);
}

void bst_inorder(BST* tree, int* arr, int* idx) {
    *idx = 0;
    inorder_traverse(tree->root, arr, idx);
}

static void preorder_traverse(BSTNode* node, int* arr, int* idx) {
    if (node == NULL) return;
    arr[(*idx)++] = node->key;
    preorder_traverse(node->left, arr, idx);
    preorder_traverse(node->right, arr, idx);
}

void bst_preorder(BST* tree, int* arr, int* idx) {
    *idx = 0;
    preorder_traverse(tree->root, arr, idx);
}

static void postorder_traverse(BSTNode* node, int* arr, int* idx) {
    if (node == NULL) return;
    postorder_traverse(node->left, arr, idx);
    postorder_traverse(node->right, arr, idx);
    arr[(*idx)++] = node->key;
}

void bst_postorder(BST* tree, int* arr, int* idx) {
    *idx = 0;
    postorder_traverse(tree->root, arr, idx);
}

static void bst_print_node(BSTNode* node, int depth) {
    if (node == NULL) return;
    bst_print_node(node->right, depth + 1);
    for (int i = 0; i < depth; i++) printf("    ");
    printf("%d\n", node->key);
    bst_print_node(node->left, depth + 1);
}

void bst_print(BST* tree) {
    bst_print_node(tree->root, 0);
}

static bool bst_is_balanced_node(BSTNode* node) {
    if (node == NULL) return true;
    int lh = bst_height_node(node->left);
    int rh = bst_height_node(node->right);
    if (abs(lh - rh) > 1) return false;
    return bst_is_balanced_node(node->left) && bst_is_balanced_node(node->right);
}

bool bst_is_balanced(BST* tree) {
    return bst_is_balanced_node(tree->root);
}
