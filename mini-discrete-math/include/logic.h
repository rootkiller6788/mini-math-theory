#ifndef LOGIC_H
#define LOGIC_H

#include <stdbool.h>

#define MAX_VARS 8
#define MAX_FORMULA 256

typedef enum {
    OP_VAR,        // 变量
    OP_NOT,        // ¬
    OP_AND,        // ∧
    OP_OR,         // ∨
    OP_IMPLIES,    // →
    OP_IFF,        // ↔
    OP_XOR         // ⊕
} OpType;

typedef struct {
    OpType op;
    union {
        int var_idx;               // OP_VAR: 变量编号 0..MAX_VARS-1
        struct { int left; int right; }; // 二元运算: 子公式索引
    };
} FormulaNode;

typedef struct {
    FormulaNode nodes[MAX_FORMULA];
    int ncount;
    char var_names[MAX_VARS];
    int nvars;
} Formula;

Formula* formula_create(void);
int formula_add_var(Formula* f, int var_idx);
int formula_add_not(Formula* f, int child);
int formula_add_and(Formula* f, int left, int right);
int formula_add_or(Formula* f, int left, int right);
int formula_add_implies(Formula* f, int left, int right);
int formula_add_iff(Formula* f, int left, int right);
int formula_add_xor(Formula* f, int left, int right);
bool formula_eval(Formula* f, int root, const bool* assignment);

void truth_table_print(Formula* f, int root);
bool is_tautology(Formula* f, int root);
bool is_contradiction(Formula* f, int root);

void proof_by_induction_demo(int n);

#endif
