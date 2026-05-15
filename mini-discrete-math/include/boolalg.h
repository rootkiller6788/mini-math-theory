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

#endif
