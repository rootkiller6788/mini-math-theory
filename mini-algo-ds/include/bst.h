#ifndef BST_H
#define BST_H

#include <stdbool.h>

typedef struct BSTNode {
    int key;
    struct BSTNode *left, *right;
} BSTNode;

typedef struct {
    BSTNode* root;
    int size;
} BST;

BST* bst_create(void);
void bst_free(BST* tree);
void bst_insert(BST* tree, int key);
bool bst_search(BST* tree, int key);
void bst_delete(BST* tree, int key);
BSTNode* bst_min(BSTNode* node);
BSTNode* bst_max(BSTNode* node);
int bst_height(BST* tree);
int bst_size(BST* tree);
void bst_inorder(BST* tree, int* arr, int* idx);
void bst_preorder(BST* tree, int* arr, int* idx);
void bst_postorder(BST* tree, int* arr, int* idx);
void bst_print(BST* tree);
bool bst_is_balanced(BST* tree);

#endif
