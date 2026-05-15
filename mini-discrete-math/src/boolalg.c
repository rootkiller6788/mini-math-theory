#include "boolalg.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

bool bool_eval_and(const bool* x) { return x[0] && x[1]; }
bool bool_eval_or(const bool* x)  { return x[0] || x[1]; }
bool bool_eval_xor(const bool* x) { return x[0] != x[1]; }
bool bool_eval_nand(const bool* x){ return !(x[0] && x[1]); }
bool bool_eval_nor(const bool* x) { return !(x[0] || x[1]); }

bool bool_eval_majority(const bool* x) {
    int sum = x[0] + x[1] + x[2];
    return sum >= 2;
}

void truth_table_generate(BoolFunction* bf) {
    int n = bf->nvars;
    int rows = 1 << n;

    for (int i = 0; i < n; i++)
        printf(" %c |", bf->var_names[i]);
    printf(" F\n");

    for (int i = 0; i < n; i++)
        printf("---|");
    printf("---\n");

    for (int r = 0; r < rows; r++) {
        bool assign[MAX_BOOL_VARS];
        for (int b = 0; b < n; b++)
            assign[b] = (r >> (n - 1 - b)) & 1;

        for (int b = 0; b < n; b++)
            printf(" %d |", assign[b]);
        printf(" %d\n", bf->func(assign));
    }
}

SOP sop_from_truth_table(BoolFunction* bf) {
    SOP sop;
    sop.n_minterms = 0;
    int n = bf->nvars;
    int rows = 1 << n;

    for (int r = 0; r < rows; r++) {
        bool assign[MAX_BOOL_VARS];
        for (int b = 0; b < n; b++)
            assign[b] = (r >> (n - 1 - b)) & 1;
        if (bf->func(assign))
            sop.minterms[sop.n_minterms++] = r;
    }
    return sop;
}

POS pos_from_truth_table(BoolFunction* bf) {
    POS pos;
    pos.n_maxterms = 0;
    int n = bf->nvars;
    int rows = 1 << n;

    for (int r = 0; r < rows; r++) {
        bool assign[MAX_BOOL_VARS];
        for (int b = 0; b < n; b++)
            assign[b] = (r >> (n - 1 - b)) & 1;
        if (!bf->func(assign))
            pos.maxterms[pos.n_maxterms++] = r;
    }
    return pos;
}

char* sop_to_string(SOP* sop, int nvars) {
    int cap = 4096;
    char* buf = malloc(cap);
    buf[0] = '\0';

    if (sop->n_minterms == 0) {
        strcpy(buf, "0");
        return buf;
    }

    for (int i = 0; i < sop->n_minterms; i++) {
        if (i > 0) strcat(buf, " + ");
        int m = sop->minterms[i];
        for (int j = 0; j < nvars; j++) {
            int val = (m >> (nvars - 1 - j)) & 1;
            if (val == 0) {
                int len = strlen(buf);
                snprintf(buf + len, cap - len, "%c'", 'A' + j);
            } else {
                int len = strlen(buf);
                snprintf(buf + len, cap - len, "%c", 'A' + j);
            }
        }
    }
    return buf;
}

char* pos_to_string(POS* pos, int nvars) {
    int cap = 4096;
    char* buf = malloc(cap);
    buf[0] = '\0';

    if (pos->n_maxterms == 0) {
        strcpy(buf, "1");
        return buf;
    }

    for (int i = 0; i < pos->n_maxterms; i++) {
        if (i > 0) strcat(buf, " * ");
        int len = strlen(buf);
        snprintf(buf + len, cap - len, "(");
        int m = pos->maxterms[i];
        for (int j = 0; j < nvars; j++) {
            int val = (m >> (nvars - 1 - j)) & 1;
            if (val == 0) {
                len = strlen(buf);
                snprintf(buf + len, cap - len, "%c", 'A' + j);
            } else {
                len = strlen(buf);
                snprintf(buf + len, cap - len, "%c'", 'A' + j);
            }
            if (j < nvars - 1) strcat(buf, " + ");
        }
        strcat(buf, ")");
    }
    return buf;
}

static int count_ones(int x) {
    int cnt = 0;
    while (x) { cnt += x & 1; x >>= 1; }
    return cnt;
}

typedef struct { int value; int mask; } Implicant;

static bool implies(int minterm, Implicant imp) {
    return (minterm & imp.mask) == (imp.value & imp.mask);
}

static bool is_valid_implicant(Implicant imp, int minterms[], int n_minterms, int nvars) {
    int fixed_bits = count_ones(imp.mask & ((1 << nvars) - 1));
    int total_cells = 1 << (nvars - fixed_bits);

    int free_bits = ((1 << nvars) - 1) ^ imp.mask;

    for (int cell = 0; cell < total_cells; cell++) {
        int m = imp.value;
        int f = free_bits;
        int c = cell;
        int pos = 0;
        while (f) {
            if (f & 1) {
                if (c & 1) m |= (1 << pos);
                c >>= 1;
            }
            f >>= 1;
            pos++;
        }
        bool in_func = false;
        for (int j = 0; j < n_minterms; j++)
            if (minterms[j] == m) { in_func = true; break; }
        if (!in_func) return false;
    }
    return true;
}

static bool is_prime_implicant(Implicant imp, int minterms[], int n_minterms, int nvars) {
    for (int bit = 0; bit < nvars; bit++) {
        if (imp.mask & (1 << bit)) {
            Implicant larger = { imp.value & ~(1 << bit), imp.mask & ~(1 << bit) };
            if (is_valid_implicant(larger, minterms, n_minterms, nvars))
                return false;
        }
    }
    return true;
}

static void implicant_to_string(Implicant imp, int nvars, char* buf) {
    buf[0] = '\0';
    bool has_literal = false;
    for (int j = 0; j < nvars; j++) {
        int bit = 1 << (nvars - 1 - j);
        if (imp.mask & bit) {
            char tmp[4];
            snprintf(tmp, 4, "%c%s", 'A' + j, (imp.value & bit) ? "" : "'");
            strcat(buf, tmp);
            has_literal = true;
        }
    }
    if (!has_literal) strcpy(buf, "1");
}

static void kmap_simplify(int nvars, int minterms[], int n_minterms, char* result) {
    result[0] = '\0';
    if (n_minterms == 0) { strcpy(result, "0"); return; }
    if (n_minterms == (1 << nvars)) { strcpy(result, "1"); return; }

    Implicant primes[256];
    int nprimes = 0;
    int max_mask = (1 << nvars) - 1;

    for (int mask = 0; mask <= max_mask; mask++) {
        for (int val = 0; val <= max_mask; val++) {
            Implicant imp = { val & mask, mask };
            if (!is_valid_implicant(imp, minterms, n_minterms, nvars)) continue;
            if (!is_prime_implicant(imp, minterms, n_minterms, nvars)) continue;
            bool dup = false;
            for (int p = 0; p < nprimes; p++)
                if (primes[p].value == imp.value && primes[p].mask == imp.mask)
                    { dup = true; break; }
            if (!dup && nprimes < 256)
                primes[nprimes++] = imp;
        }
    }

    bool covered[256] = {false};
    int remaining = n_minterms;

    while (remaining > 0) {
        int best_idx = -1, best_cnt = 0;
        for (int p = 0; p < nprimes; p++) {
            int cnt = 0;
            for (int i = 0; i < n_minterms; i++)
                if (!covered[i] && implies(minterms[i], primes[p]))
                    cnt++;
            if (cnt > best_cnt) { best_cnt = cnt; best_idx = p; }
        }
        if (best_idx < 0 || best_cnt == 0) break;

        if (result[0] != '\0') strcat(result, " + ");
        char term[64];
        implicant_to_string(primes[best_idx], nvars, term);
        strcat(result, term);

        for (int i = 0; i < n_minterms; i++)
            if (!covered[i] && implies(minterms[i], primes[best_idx]))
                { covered[i] = true; remaining--; }
    }
}

void karnaugh_map_2var(BoolFunction* bf, char* result) {
    SOP sop = sop_from_truth_table(bf);
    kmap_simplify(2, sop.minterms, sop.n_minterms, result);
}

void karnaugh_map_3var(BoolFunction* bf, char* result) {
    SOP sop = sop_from_truth_table(bf);
    kmap_simplify(3, sop.minterms, sop.n_minterms, result);
}

void karnaugh_map_4var(BoolFunction* bf, char* result) {
    SOP sop = sop_from_truth_table(bf);
    kmap_simplify(4, sop.minterms, sop.n_minterms, result);
}

void circuit_init(LogicCircuit* c, int ninputs) {
    c->ngates = 0;
    c->ninputs = ninputs;
    c->output_id = -1;
    memset(c->gates, 0, sizeof(c->gates));
}

int circuit_add_gate(LogicCircuit* c, GateType type, int in1, int in2) {
    c->gates[c->ngates].type = type;
    c->gates[c->ngates].input1 = in1;
    c->gates[c->ngates].input2 = in2;
    c->gates[c->ngates].nid = c->ngates;
    c->output_id = c->ngates;
    return c->ngates++;
}

void circuit_evaluate(LogicCircuit* c, const bool* inputs, bool* outputs, int ngates) {
    for (int i = 0; i < c->ngates; i++) {
        bool in1 = false, in2 = false;
        if (c->gates[i].input1 >= 0)
            in1 = ((c->gates[i].input1 < c->ninputs) ?
                   inputs[c->gates[i].input1] :
                   outputs[c->gates[i].input1 - c->ninputs]);
        if (c->gates[i].input2 >= 0)
            in2 = ((c->gates[i].input2 < c->ninputs) ?
                   inputs[c->gates[i].input2] :
                   outputs[c->gates[i].input2 - c->ninputs]);

        switch (c->gates[i].type) {
            case GATE_AND:  outputs[i] = in1 && in2; break;
            case GATE_OR:   outputs[i] = in1 || in2; break;
            case GATE_NOT:  outputs[i] = !in1; break;
            case GATE_NAND: outputs[i] = !(in1 && in2); break;
            case GATE_NOR:  outputs[i] = !(in1 || in2); break;
            case GATE_XOR:  outputs[i] = in1 != in2; break;
        }
    }
}
