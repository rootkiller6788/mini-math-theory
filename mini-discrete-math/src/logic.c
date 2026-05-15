#include "logic.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Formula* formula_create(void) {
    Formula* f = malloc(sizeof(Formula));
    f->ncount = 0;
    f->nvars = 0;
    return f;
}

int formula_add_var(Formula* f, int var_idx) {
    FormulaNode node = { .op = OP_VAR, .var_idx = var_idx };
    f->nodes[f->ncount] = node;
    return f->ncount++;
}

int formula_add_not(Formula* f, int child) {
    FormulaNode node = { .op = OP_NOT, .left = child };
    f->nodes[f->ncount] = node;
    return f->ncount++;
}

int formula_add_and(Formula* f, int left, int right) {
    FormulaNode node = { .op = OP_AND, .left = left, .right = right };
    f->nodes[f->ncount] = node;
    return f->ncount++;
}

int formula_add_or(Formula* f, int left, int right) {
    FormulaNode node = { .op = OP_OR, .left = left, .right = right };
    f->nodes[f->ncount] = node;
    return f->ncount++;
}

int formula_add_implies(Formula* f, int left, int right) {
    FormulaNode node = { .op = OP_IMPLIES, .left = left, .right = right };
    f->nodes[f->ncount] = node;
    return f->ncount++;
}

int formula_add_iff(Formula* f, int left, int right) {
    FormulaNode node = { .op = OP_IFF, .left = left, .right = right };
    f->nodes[f->ncount] = node;
    return f->ncount++;
}

int formula_add_xor(Formula* f, int left, int right) {
    FormulaNode node = { .op = OP_XOR, .left = left, .right = right };
    f->nodes[f->ncount] = node;
    return f->ncount++;
}

bool formula_eval(Formula* f, int root, const bool* assignment) {
    FormulaNode* node = &f->nodes[root];
    switch (node->op) {
        case OP_VAR: return assignment[node->var_idx];
        case OP_NOT: return !formula_eval(f, node->left, assignment);
        case OP_AND: return formula_eval(f, node->left, assignment) && formula_eval(f, node->right, assignment);
        case OP_OR:  return formula_eval(f, node->left, assignment) || formula_eval(f, node->right, assignment);
        case OP_IMPLIES: return !formula_eval(f, node->left, assignment) || formula_eval(f, node->right, assignment);
        case OP_IFF: return formula_eval(f, node->left, assignment) == formula_eval(f, node->right, assignment);
        case OP_XOR: return formula_eval(f, node->left, assignment) != formula_eval(f, node->right, assignment);
    }
    return false;
}

void truth_table_print(Formula* f, int root) {
    int n = f->nvars;
    int rows = 1 << n;

    for (int i = 0; i < n; i++)
        printf(" %c |", f->var_names[i]);
    printf(" R\n");

    for (int i = 0; i < n; i++)
        printf("---|");
    printf("---\n");

    for (int r = 0; r < rows; r++) {
        bool assign[MAX_VARS];
        for (int b = 0; b < n; b++)
            assign[b] = (r >> (n - 1 - b)) & 1;

        for (int b = 0; b < n; b++)
            printf(" %d |", assign[b]);
        printf(" %d\n", formula_eval(f, root, assign));
    }
}

bool is_tautology(Formula* f, int root) {
    int rows = 1 << f->nvars;
    for (int r = 0; r < rows; r++) {
        bool assign[MAX_VARS];
        for (int b = 0; b < f->nvars; b++)
            assign[b] = (r >> (f->nvars - 1 - b)) & 1;
        if (!formula_eval(f, root, assign))
            return false;
    }
    return true;
}

bool is_contradiction(Formula* f, int root) {
    int rows = 1 << f->nvars;
    for (int r = 0; r < rows; r++) {
        bool assign[MAX_VARS];
        for (int b = 0; b < f->nvars; b++)
            assign[b] = (r >> (f->nvars - 1 - b)) & 1;
        if (formula_eval(f, root, assign))
            return false;
    }
    return true;
}

void proof_by_induction_demo(int n) {
    printf("=== 数学归纳法演示 ===\n");
    printf("命题 P(n): 1 + 2 + ... + n = n(n+1)/2\n\n");

    printf("1. 基始: P(1)\n");
    printf("   左 = 1, 右 = 1*2/2 = 1  ✓\n\n");

    printf("2. 归纳: 假设 P(k) 成立, 证 P(k+1)\n");
    printf("   1+2+...+k+(k+1) = k(k+1)/2 + (k+1)\n");
    printf("   = (k+1)(k/2 + 1) = (k+1)(k+2)/2  ✓\n\n");

    printf("3. 验证若干项:\n");
    int sum = 0;
    for (int i = 1; i <= n && i <= 10; i++) {
        sum += i;
        printf("   n=%d: 和=%d, 公式=%d\n", i, sum, i * (i + 1) / 2);
    }
    printf("\n");
}
