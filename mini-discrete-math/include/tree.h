#ifndef TREE_H
#define TREE_H

#include <stdbool.h>

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

#endif
