#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    printf("====== 05 树结构 ======\n\n");

    // 1. BST
    printf("--- 二叉搜索树 (BST) ---\n");
    TreeNode* bst = NULL;
    int keys[] = {50, 30, 70, 20, 40, 60, 80};
    for (int i = 0; i < 7; i++)
        bst = bst_insert(bst, keys[i]);

    printf("BST 结构:\n");
    tree_print(bst, 0);

    int arr[256], idx = 0;
    tree_inorder(bst, arr, &idx);
    printf("中序遍历: ");
    for (int i = 0; i < idx; i++) printf("%d ", arr[i]);
    printf("\n");

    idx = 0; tree_preorder(bst, arr, &idx);
    printf("前序遍历: ");
    for (int i = 0; i < idx; i++) printf("%d ", arr[i]);
    printf("\n");

    idx = 0; tree_postorder(bst, arr, &idx);
    printf("后序遍历: ");
    for (int i = 0; i < idx; i++) printf("%d ", arr[i]);
    printf("\n");

    idx = 0; tree_levelorder(bst, arr, &idx);
    printf("层序遍历: ");
    for (int i = 0; i < idx; i++) printf("%d ", arr[i]);
    printf("\n");

    printf("高度: %d, 节点数: %d\n", tree_height(bst), tree_size(bst));
    printf("是 BST? %s\n\n", tree_is_bst(bst, -999999, 999999) ? "是" : "否");

    // 删除操作
    printf("--- BST 删除 50 ---\n");
    bst = bst_delete(bst, 50);
    tree_print(bst, 0);
    idx = 0; tree_inorder(bst, arr, &idx);
    printf("中序: ");
    for (int i = 0; i < idx; i++) printf("%d ", arr[i]);
    printf("\n\n");

    tree_free(bst);

    // 2. AVL
    printf("--- AVL 树 ---\n");
    TreeNode* avl = NULL;
    int avl_keys[] = {10, 20, 30, 40, 50, 25};
    for (int i = 0; i < 6; i++) {
        avl = avl_insert(avl, avl_keys[i]);
        printf("插入 %d 后高度: %d\n", avl_keys[i], tree_height(avl));
    }
    tree_print(avl, 0);
    printf("AVL 高度: %d, 是 BST? %s\n\n",
           tree_height(avl), tree_is_bst(avl, -999999, 999999) ? "是" : "否");
    tree_free(avl);

    // 3. Huffman 编码
    printf("--- Huffman 编码 ---\n");
    char* text = "this is a simple huffman coding example";

    int freq[256];
    HuffmanNode* hroot = huffman_build_tree(text, freq);

    HuffmanCode codes[256] = {0};
    char code_buf[256];
    huffman_generate_codes(hroot, code_buf, 0, codes);

    printf("字符频率与编码:\n");
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            printf("  '%c': freq=%d, code=", (char)i, freq[i]);
            for (int j = 0; j < codes[i].code_len; j++)
                printf("%c", codes[i].code[j]);
            printf(" (%d bits)\n", codes[i].code_len);
        }
    }

    char* encoded = huffman_encode(text, codes);
    char* decoded = huffman_decode(encoded, hroot);

    printf("\n原文:   %s\n", text);
    printf("编码:   %s\n", encoded);
    printf("解码:   %s\n", decoded);
    printf("原长度: %zu bytes, 编码后: %zu bits\n",
           strlen(text), strlen(encoded));

    printf("验证: %s\n\n", strcmp(text, decoded) == 0 ? "通过 ✓" : "失败 ✗");

    free(encoded);
    free(decoded);
    huffman_free(hroot);

    return 0;
}
