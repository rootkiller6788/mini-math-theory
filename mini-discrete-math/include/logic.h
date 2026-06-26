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

/* ===== CNF (Conjunctive Normal Form) ===== */
/* Clause: disjunction of literals. Literal < 0 means negated. */
#define MAX_CLAUSES 64
#define MAX_LITS_PER_CLAUSE 8

typedef struct {
    int lits[MAX_LITS_PER_CLAUSE];
    int n_lits;
} Clause;

typedef struct {
    Clause clauses[MAX_CLAUSES];
    int n_clauses;
    int n_vars;
} CNF;

/* Convert propositional formula tree to CNF via Tseitin transformation */
CNF* formula_to_cnf(Formula* f, int root);

/* DPLL SAT solver: Davis-Putnam-Logemann-Loveland
 * Returns true if CNF is satisfiable, fills model array.
 * Algorithm: unit propagation + pure literal elimination + splitting.
 * Reference: Davis-Logemann-Loveland (1962), MIT 6.042J */
bool dpll_solve(CNF* cnf, bool* model);

/* Resolution proof: derive empty clause to prove UNSAT
 * Returns true if clause C can be derived from clauses (refutation complete).
 * Reference: Robinson (1965), Resolution Principle */
bool resolution_prove(CNF* cnf_from, int* goal_lits, int goal_nlits);

/* Horn formula satisfiability — linear time
 * Horn clause: at most one positive literal.
 * Algorithm: forward chaining (mark true all forced positives).
 * Reference: Dowling-Gallier (1984), MIT 6.034 */
bool horn_sat_solve(CNF* cnf, bool* model);

/* 2-SAT: implication graph + SCC check for O(n+m)
 * Each clause (a ∨ b) becomes (¬a → b) and (¬b → a).
 * Reference: Aspvall-Plass-Tarjan (1979), MIT 6.046J */
bool two_sat_solve(int n_vars, int clauses[][2], int n_clauses, bool* model);

/* Semantic tableau: systematic search for counter-model */
bool semantic_tableau_check(Formula* f, int root);

/* Natural deduction proof checker for simple propositional proofs */
typedef enum {
    RULE_ASSUME, RULE_AND_INTRO, RULE_AND_ELIM1, RULE_AND_ELIM2,
    RULE_OR_INTRO1, RULE_OR_INTRO2, RULE_OR_ELIM,
    RULE_IMPLIES_INTRO, RULE_IMPLIES_ELIM,  /* →I, →E = modus ponens */
    RULE_NOT_INTRO, RULE_NOT_ELIM,
    RULE_IFF_INTRO, RULE_IFF_ELIM1, RULE_IFF_ELIM2,
    RULE_FALSE_ELIM, RULE_TRUE_INTRO
} NDRule;

typedef struct {
    NDRule rule;
    int premises[3];  /* premise line numbers (-1 = none) */
    int conclusion;   /* formula root index in Formula */
    int n_premises;
} NDStep;

typedef struct {
    Formula* formula;
    NDStep steps[128];
    int n_steps;
    int assumptions[16];  /* line numbers of open assumptions */
    int n_assumptions;
} NDProof;

NDProof* nd_proof_create(Formula* f);
int nd_add_step(NDProof* p, NDRule rule, int prem1, int prem2, int prem3, int conc);
bool nd_verify_proof(NDProof* p);

#endif
