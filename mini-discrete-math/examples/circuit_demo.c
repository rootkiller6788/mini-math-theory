#include "boolalg.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

bool majority_3(const bool* x) {
    int s = x[0] + x[1] + x[2];
    return s >= 2;
}

bool func_sample(const bool* x) {
    return (!x[0] && !x[1] && x[2]) ||
           (!x[0] && x[1] && x[2])  ||
           (x[0] && !x[1] && !x[2]) ||
           (x[0] && !x[1] && x[2])  ||
           (x[0] && x[1] && x[2]);
}

bool func_4var(const bool* x) {
    int idx = x[0]*8 + x[1]*4 + x[2]*2 + x[3]*1;
    return idx == 0 || idx == 1 || idx == 2 || idx == 5 ||
           idx == 8 || idx == 9 || idx == 10;
}

int main(void) {
    printf("====== 07 布尔代数 ======\n\n");

    printf("--- 真值表: 2变量 XOR ---\n");
    BoolFunction bf_xor = { .func = bool_eval_xor, .nvars = 2,
                            .var_names = {'A', 'B'} };
    truth_table_generate(&bf_xor);
    printf("\n");

    printf("--- 3变量多数表决 ---\n");
    BoolFunction bf_maj = { .func = majority_3, .nvars = 3,
                            .var_names = {'A', 'B', 'C'} };
    truth_table_generate(&bf_maj);
    printf("\n");

    printf("--- SOP / POS ---\n");
    BoolFunction bf_sample = { .func = func_sample, .nvars = 3,
                               .var_names = {'A', 'B', 'C'} };
    truth_table_generate(&bf_sample);

    SOP sop = sop_from_truth_table(&bf_sample);
    char* sop_str = sop_to_string(&sop, 3);
    printf("SOP: F = %s\n", sop_str);

    POS pos = pos_from_truth_table(&bf_sample);
    char* pos_str = pos_to_string(&pos, 3);
    printf("POS: F = %s\n", pos_str);
    free(sop_str);
    free(pos_str);
    printf("\n");

    printf("--- 卡诺图化简 ---\n");
    char simplified[256];

    BoolFunction bf_or = { .func = bool_eval_or, .nvars = 2,
                           .var_names = {'A', 'B'} };
    karnaugh_map_2var(&bf_or, simplified);
    printf("K-map 2-var OR:  F = %s\n", simplified);

    karnaugh_map_3var(&bf_maj, simplified);
    printf("K-map 3-var maj: F = %s\n", simplified);

    BoolFunction bf4 = { .func = func_4var, .nvars = 4 };
    karnaugh_map_4var(&bf4, simplified);
    printf("K-map 4-var:     F = %s\n\n", simplified);

    printf("--- 逻辑电路仿真 ---\n");

    printf("半加器 (Half Adder):\n");
    LogicCircuit ha;
    circuit_init(&ha, 2);
    int ha_xor = circuit_add_gate(&ha, GATE_XOR, 0, 1);
    int ha_and = circuit_add_gate(&ha, GATE_AND, 0, 1);

    printf("  A B | S C\n");
    printf("  ----|----\n");
    for (int i = 0; i < 4; i++) {
        bool in[2] = { (i>>1)&1, i&1 };
        bool out[256];
        circuit_evaluate(&ha, in, out, 2);
        printf("  %d %d | %d %d\n", in[0], in[1], out[ha_xor], out[ha_and]);
    }
    printf("\n");

    printf("全加器 (Full Adder):\n");
    LogicCircuit fa;
    circuit_init(&fa, 3);
    int g_xor = circuit_add_gate(&fa, GATE_XOR, 0, 1);
    int sum   = circuit_add_gate(&fa, GATE_XOR, fa.ninputs + g_xor, 2);
    int g_and1 = circuit_add_gate(&fa, GATE_AND, 0, 1);
    int g_and2 = circuit_add_gate(&fa, GATE_AND, fa.ninputs + g_xor, 2);
    int carry  = circuit_add_gate(&fa, GATE_OR, fa.ninputs + g_and1, fa.ninputs + g_and2);

    printf("  A B Ci| S Co\n");
    printf("  ------|-----\n");
    for (int i = 0; i < 8; i++) {
        bool in[3] = { (i>>2)&1, (i>>1)&1, i&1 };
        bool out[256];
        circuit_evaluate(&fa, in, out, 6);
        printf("  %d %d %d | %d %d\n", in[0], in[1], in[2], out[sum], out[carry]);
    }

    return 0;
}
