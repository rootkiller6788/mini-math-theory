#ifndef BOOLALG_H
#define BOOLALG_H

#include <stdbool.h>

#define MAX_BOOL_VARS 6

typedef struct {
    bool (*func)(const bool*);
    int nvars;
    char var_names[MAX_BOOL_VARS];
} BoolFunction;

bool bool_eval_and(const bool* x);
bool bool_eval_or(const bool* x);
bool bool_eval_xor(const bool* x);
bool bool_eval_nand(const bool* x);
bool bool_eval_nor(const bool* x);
bool bool_eval_majority(const bool* x);

void truth_table_generate(BoolFunction* bf);

typedef struct {
    int minterms[1 << MAX_BOOL_VARS];
    int n_minterms;
} SOP;

typedef struct {
    int maxterms[1 << MAX_BOOL_VARS];
    int n_maxterms;
} POS;

SOP sop_from_truth_table(BoolFunction* bf);
POS pos_from_truth_table(BoolFunction* bf);
char* sop_to_string(SOP* sop, int nvars);
char* pos_to_string(POS* pos, int nvars);

void karnaugh_map_2var(BoolFunction* bf, char* result);
void karnaugh_map_3var(BoolFunction* bf, char* result);
void karnaugh_map_4var(BoolFunction* bf, char* result);

typedef enum {
    GATE_AND, GATE_OR, GATE_NOT, GATE_NAND, GATE_NOR, GATE_XOR
} GateType;

typedef struct GateNode {
    GateType type;
    int input1, input2;  // -1 means primary input
    int nid;
} GateNode;

typedef struct {
    GateNode gates[256];
    int ngates;
    int ninputs;
    int output_id;
} LogicCircuit;

void circuit_init(LogicCircuit* circuit, int ninputs);
int circuit_add_gate(LogicCircuit* circuit, GateType type, int in1, int in2);
void circuit_evaluate(LogicCircuit* circuit, const bool* inputs, bool* outputs, int ngates);

/* Quine-McCluskey with don't-care terms */
typedef struct {
    int minterms[1 << MAX_BOOL_VARS];
    int n_minterms;
    int dont_cares[1 << MAX_BOOL_VARS];
    int n_dont_cares;
    int nvars;
} QMInput;

typedef struct {
    int terms[256][MAX_BOOL_VARS];   /* -1=absent, 0=0, 1=1 */
    int n_terms;
} QMOutput;

/* Full Quine-McCluskey algorithm */
QMOutput quine_mccluskey_solve(QMInput* input);
char* qm_output_to_string(QMOutput* out, int nvars);

/* Multi-level logic optimization: factor common sub-expressions */
typedef struct {
    char expr[1024];   /* simplified multi-level expression */
    int literal_count;  /* # literals in optimized form */
} MLLevelOpt;

MLLevelOpt multi_level_optimize(SOP* sop, int nvars);

/* Binary Decision Diagram (BDD) — reduced ordered BDD basics */
#define BDD_MAX_NODES 1024

typedef struct BDDNode {
    int var;             /* variable index (0..nvars-1), or -1 for terminal */
    int low;             /* child when var=0 (node index or 0/1 constant) */
    int high;            /* child when var=1 */
    int id;
} BDDNode;

typedef struct {
    BDDNode nodes[BDD_MAX_NODES];
    int n_nodes;
    int nvars;
    int var_order[MAX_BOOL_VARS]; /* variable ordering */
} BDD;

BDD* bdd_create(int nvars);
int bdd_make_node(BDD* bdd, int var, int low, int high);
int bdd_apply_and(BDD* bdd, int f, int g);
int bdd_apply_or(BDD* bdd, int f, int g);
int bdd_apply_not(BDD* bdd, int f);
int bdd_from_truth_table(BDD* bdd, BoolFunction* bf);
bool bdd_evaluate(BDD* bdd, int root, const bool* assign);
int bdd_sat_count(BDD* bdd, int root);
void bdd_free(BDD* bdd);

/* Boolean function equivalence checking via SAT */
bool bool_equiv_check(BoolFunction* a, BoolFunction* b);

/* Functional completeness: check if gate set is functionally complete */
bool is_functionally_complete(GateType gates[], int n_gates);

/* Shannon's Expansion: f = (x ∧ f_x) ∨ (¬x ∧ f_¬x) */
void shannon_expand(BoolFunction* bf, int var, BoolFunction* f_positive, BoolFunction* f_negative);

/* Boolean difference ∂f/∂x: f(x=0) ⊕ f(x=1) */
bool boolean_derivative(BoolFunction* bf, int var, const bool* assign);

/* Rademacher-Walsh spectrum of boolean function */
void walsh_spectrum(BoolFunction* bf, int spectrum[]);

#endif
