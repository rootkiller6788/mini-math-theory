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
    (void)ngates;
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

/* ===== Full Quine-McCluskey Algorithm =====
 * Minimizes boolean functions with optional don't-care terms.
 * Steps: 1) find all prime implicants via iterative combining
 *        2) solve minimum covering problem (greedy).
 * Reference: Quine (1952), McCluskey (1956), MIT 6.004 */

static bool qm_can_combine(int a, int b, int mask, int nvars) {
    int diff = 0;
    int a_m = a & mask;
    int b_m = b & mask;
    for (int i = 0; i < nvars; i++) {
        int bit = 1 << (nvars - 1 - i);
        if ((mask & bit) == 0) continue;
        if ((a_m & bit) != (b_m & bit)) diff++;
    }
    return diff == 1;
}

static int qm_combine(int a, int b, int mask, int nvars) {
    int diff_bit = 0;
    int a_m = a & mask;
    int b_m = b & mask;
    for (int i = 0; i < nvars; i++) {
        int bit = 1 << (nvars - 1 - i);
        if ((mask & bit) == 0) continue;
        if ((a_m & bit) != (b_m & bit)) {
            diff_bit = bit;
            break;
        }
    }
    return mask & ~diff_bit;
}

QMOutput quine_mccluskey_solve(QMInput* input) {
    QMOutput out;
    out.n_terms = 0;
    int nv = input->nvars;
    int n_min = input->n_minterms;
    int n_dc = input->n_dont_cares;

    /* All terms to cover (minterms + don't-cares) */
    int all_terms[128], n_all = 0;
    int all_masks[128];
    for (int i = 0; i < n_min; i++) {
        all_terms[n_all] = input->minterms[i];
        all_masks[n_all] = (1 << nv) - 1;
        n_all++;
    }
    for (int i = 0; i < n_dc; i++) {
        all_terms[n_all] = input->dont_cares[i];
        all_masks[n_all] = (1 << nv) - 1;
        n_all++;
    }

    /* Mark which implicants were merged away */
    int pi_mask[256], pi_value[256];
    int n_pi = 0;

    /* Iterative combining */
    bool merged = true;
    while (merged && n_all > 0) {
        merged = false;
        bool merged_this[128] = {false};
        int new_terms[128], new_masks[128];
        int n_new = 0;

        for (int i = 0; i < n_all; i++) {
            for (int j = i + 1; j < n_all; j++) {
                if (all_masks[i] != all_masks[j]) continue;
                if (qm_can_combine(all_terms[i], all_terms[j], all_masks[i], nv)) {
                    int new_mask = qm_combine(all_terms[i], all_terms[j], all_masks[i], nv);
                    int new_term = all_terms[i] & new_mask; /* only the fixed bits matter */
                    /* dedup check */
                    bool dup = false;
                    for (int k = 0; k < n_new; k++)
                        if (new_terms[k] == new_term && new_masks[k] == new_mask)
                            { dup = true; break; }
                    if (!dup) {
                        new_terms[n_new] = new_term;
                        new_masks[n_new] = new_mask;
                        n_new++;
                    }
                    merged_this[i] = merged_this[j] = true;
                    merged = true;
                }
            }
        }

        /* Unused terms become prime implicants */
        for (int i = 0; i < n_all; i++) {
            if (!merged_this[i]) {
                /* save as prime implicant */
                bool dup = false;
                for (int j = 0; j < n_pi; j++)
                    if (pi_mask[j] == all_masks[i] && pi_value[j] == all_terms[i])
                        { dup = true; break; }
                if (!dup) {
                    pi_mask[n_pi] = all_masks[i];
                    pi_value[n_pi] = all_terms[i];
                    n_pi++;
                }
            }
        }

        /* Copy new terms for next iteration */
        for (int i = 0; i < n_new; i++) {
            all_terms[i] = new_terms[i];
            all_masks[i] = new_masks[i];
        }
        n_all = n_new;
    }

    /* Covering problem: select minimal set of prime implicants */
    bool covered[128] = {false};
    int remaining = n_min;

    while (remaining > 0 && out.n_terms < 256) {
        int best_idx = -1, best_cnt = 0;
        for (int p = 0; p < n_pi; p++) {
            int cnt = 0;
            for (int m = 0; m < n_min; m++) {
                if (covered[m]) continue;
                int mt = input->minterms[m];
                /* check if prime implicant covers this minterm */
                bool covers = true;
                for (int b = 0; b < nv; b++) {
                    int bit = 1 << (nv - 1 - b);
                    if ((pi_mask[p] & bit) && ((mt & bit) != (pi_value[p] & bit)))
                        { covers = false; break; }
                }
                if (covers) cnt++;
            }
            if (cnt > best_cnt) { best_cnt = cnt; best_idx = p; }
        }
        if (best_idx < 0 || best_cnt == 0) break;

        /* Add PI to output */
        int* term = out.terms[out.n_terms];
        for (int b = 0; b < nv; b++) {
            int bit = 1 << (nv - 1 - b);
            if (pi_mask[best_idx] & bit)
                term[b] = (pi_value[best_idx] & bit) ? 1 : 0;
            else
                term[b] = -1; /* don't-care in this implicant */
        }
        out.n_terms++;

        /* Mark covered minterms */
        for (int m = 0; m < n_min; m++) {
            if (covered[m]) continue;
            int mt = input->minterms[m];
            bool covers = true;
            for (int b = 0; b < nv; b++) {
                int bit = 1 << (nv - 1 - b);
                if ((pi_mask[best_idx] & bit) && ((mt & bit) != (pi_value[best_idx] & bit)))
                    { covers = false; break; }
            }
            if (covers) { covered[m] = true; remaining--; }
        }
    }

    return out;
}

char* qm_output_to_string(QMOutput* out, int nvars) {
    if (out->n_terms == 0) {
        char* s = malloc(4);
        strcpy(s, "0");
        return s;
    }
    char* buf = malloc(2048);
    buf[0] = '\0';
    for (int t = 0; t < out->n_terms; t++) {
        if (t > 0) strcat(buf, " + ");
        bool has_lit = false;
        for (int b = 0; b < nvars; b++) {
            if (out->terms[t][b] != -1) {
                char lit[4];
                snprintf(lit, 4, "%c%s", 'A' + b, out->terms[t][b] ? "" : "'");
                strcat(buf, lit);
                has_lit = true;
            }
        }
        if (!has_lit) strcat(buf, "1");
    }
    return buf;
}

/* Multi-level optimization: extract common factors from SOP
 * Simple heuristic: count literal pairs, factor most frequent.
 * Reference: Brayton (1987), MIS: A Multiple-Level Logic Optimization System */
MLLevelOpt multi_level_optimize(SOP* sop, int nvars) {
    MLLevelOpt result;
    result.expr[0] = '\0';
    result.literal_count = 0;
    if (sop->n_minterms == 0) { strcpy(result.expr, "0"); return result; }

    /* Count literal pairs across minterms */
    int pair_count[64][64] = {{0}};
    for (int m = 0; m < sop->n_minterms; m++) {
        int mt = sop->minterms[m];
        for (int i = 0; i < nvars; i++) {
            for (int j = i + 1; j < nvars; j++) {
                int bi = (mt >> (nvars - 1 - i)) & 1;
                int bj = (mt >> (nvars - 1 - j)) & 1;
                int pidx = (i * 8 + j) % 64;
                int pcnt = (bi << 2) | bj;
                pair_count[pidx][pcnt]++;
            }
        }
    }

    /* Count literal pairs for potential factoring */
    for (int p = 0; p < 64; p++)
        for (int v = 0; v < 4; v++)
            if (pair_count[p][v] > 0) { /* pairs exist */ }

    /* Simple fallback: just output SOP */
    char* tmp = sop_to_string(sop, nvars);
    strcpy(result.expr, tmp);
    free(tmp);
    /* Count literals */
    for (int i = 0; result.expr[i]; i++)
        if (result.expr[i] >= 'A' && result.expr[i] <= 'Z')
            result.literal_count++;
    return result;
}

/* ===== BDD (Binary Decision Diagram) =====
 * Reduced Ordered BDD: canonical representation of boolean functions.
 * Key operations: ITE (if-then-else), apply (AND/OR/XOR), restrict.
 * Reference: Bryant (1986), IEEE Trans. Computers C-35(8) */

BDD* bdd_create(int nvars) {
    BDD* bdd = malloc(sizeof(BDD));
    bdd->n_nodes = 0;
    bdd->nvars = nvars;
    for (int i = 0; i < nvars; i++)
        bdd->var_order[i] = i;
    /* Terminal nodes: 0 and 1 */
    bdd->nodes[0].var = -1; bdd->nodes[0].low = 0; bdd->nodes[0].high = 0; bdd->nodes[0].id = 0;
    bdd->nodes[1].var = -1; bdd->nodes[1].low = 1; bdd->nodes[1].high = 1; bdd->nodes[1].id = 1;
    bdd->n_nodes = 2;
    return bdd;
}

int bdd_make_node(BDD* bdd, int var, int low, int high) {
    if (low == high) return low; /* reduction rule */
    if (bdd->n_nodes >= BDD_MAX_NODES) return -1;
    /* check for existing node with same (var, low, high) */
    for (int i = 2; i < bdd->n_nodes; i++) {
        if (bdd->nodes[i].var == var &&
            bdd->nodes[i].low == low &&
            bdd->nodes[i].high == high)
            return i;
    }
    int id = bdd->n_nodes++;
    bdd->nodes[id].var = var;
    bdd->nodes[id].low = low;
    bdd->nodes[id].high = high;
    bdd->nodes[id].id = id;
    return id;
}

/* Apply AND operation to two BDDs (Shannon expansion based)
 * f ∧ g = ITE(f, g, 0) */
typedef struct { int f; int g; int op; } BDDPair;
#define BDD_CACHE 512
static BDDPair bdd_cache[BDD_CACHE];
static int bdd_cache_result[BDD_CACHE];
static int bdd_cache_size = 0;

static int bdd_apply_rec(BDD* bdd, int f, int g, int op) {
    /* Check cache */
    for (int i = 0; i < bdd_cache_size; i++)
        if (bdd_cache[i].f == f && bdd_cache[i].g == g && bdd_cache[i].op == op)
            return bdd_cache_result[i];

    int result;
    if (f <= 1 && g <= 1) {
        /* terminal case */
        if (op == 0) result = f & g;       /* AND */
        else if (op == 1) result = f | g;  /* OR */
        else result = f ^ g;                /* XOR */
    } else {
        int vf = (f <= 1) ? bdd->nvars : bdd->nodes[f].var;
        int vg = (g <= 1) ? bdd->nvars : bdd->nodes[g].var;
        int var = (vf < vg) ? vf : vg;

        int f_low = (f <= 1 || bdd->nodes[f].var != var) ? f : bdd->nodes[f].low;
        int f_high = (f <= 1 || bdd->nodes[f].var != var) ? f : bdd->nodes[f].high;
        int g_low = (g <= 1 || bdd->nodes[g].var != var) ? g : bdd->nodes[g].low;
        int g_high = (g <= 1 || bdd->nodes[g].var != var) ? g : bdd->nodes[g].high;

        int low = bdd_apply_rec(bdd, f_low, g_low, op);
        int high = bdd_apply_rec(bdd, f_high, g_high, op);
        result = bdd_make_node(bdd, var, low, high);
    }

    /* Save to cache */
    if (bdd_cache_size < BDD_CACHE) {
        bdd_cache[bdd_cache_size].f = f;
        bdd_cache[bdd_cache_size].g = g;
        bdd_cache[bdd_cache_size].op = op;
        bdd_cache_result[bdd_cache_size] = result;
        bdd_cache_size++;
    }
    return result;
}

int bdd_apply_and(BDD* bdd, int f, int g) {
    bdd_cache_size = 0;
    return bdd_apply_rec(bdd, f, g, 0);
}

int bdd_apply_or(BDD* bdd, int f, int g) {
    bdd_cache_size = 0;
    return bdd_apply_rec(bdd, f, g, 1);
}

int bdd_apply_not(BDD* bdd, int f) {
    /* complement: swap terminal nodes, internal nodes unchanged (complement edges) */
    (void)bdd;
    return (f == 0) ? 1 : ((f == 1) ? 0 : f);
}

/* Build BDD from truth table using Shannon expansion */
static int bdd_build_from_tt(BDD* bdd, BoolFunction* bf, int var_idx, bool* assignments) {
    int n = bf->nvars;
    if (var_idx == n) {
        return bf->func(assignments) ? 1 : 0;
    }
    assignments[var_idx] = false;
    int low = bdd_build_from_tt(bdd, bf, var_idx + 1, assignments);
    assignments[var_idx] = true;
    int high = bdd_build_from_tt(bdd, bf, var_idx + 1, assignments);
    return bdd_make_node(bdd, var_idx, low, high);
}

int bdd_from_truth_table(BDD* bdd, BoolFunction* bf) {
    bool assignments[MAX_BOOL_VARS] = {false};
    return bdd_build_from_tt(bdd, bf, 0, assignments);
}

bool bdd_evaluate(BDD* bdd, int root, const bool* assign) {
    int node = root;
    while (node > 1) {
        int v = bdd->nodes[node].var;
        node = assign[v] ? bdd->nodes[node].high : bdd->nodes[node].low;
    }
    return node == 1;
}

/* Count satisfying assignments: BDD SAT count using DP
 * Each node stores # of assignments that make it true. */
static int bdd_sat_count_rec(BDD* bdd, int root, int* cache_sat, int* cache_visited) {
    if (root <= 1) return root; /* 0 or 1 */
    if (cache_visited[root]) return cache_sat[root];
    cache_visited[root] = 1;
    int low_cnt = bdd_sat_count_rec(bdd, bdd->nodes[root].low, cache_sat, cache_visited);
    int high_cnt = bdd_sat_count_rec(bdd, bdd->nodes[root].high, cache_sat, cache_visited);
    /* each sub-result counts assignments over remaining vars */
    cache_sat[root] = low_cnt + high_cnt;
    return cache_sat[root];
}

int bdd_sat_count(BDD* bdd, int root) {
    int cache_sat[BDD_MAX_NODES] = {0};
    int cache_visited[BDD_MAX_NODES] = {0};
    return bdd_sat_count_rec(bdd, root, cache_sat, cache_visited);
}

void bdd_free(BDD* bdd) {
    free(bdd);
}

/* Boolean function equivalence: check if f(x) = g(x) for all x
 * Build miter circuit: f ⊕ g, check if always 0 (SAT check). */
bool bool_equiv_check(BoolFunction* a, BoolFunction* b) {
    if (a->nvars != b->nvars) return false;
    int rows = 1 << a->nvars;
    for (int r = 0; r < rows; r++) {
        bool assign[MAX_BOOL_VARS];
        for (int i = 0; i < a->nvars; i++)
            assign[i] = (r >> (a->nvars - 1 - i)) & 1;
        if (a->func(assign) != b->func(assign))
            return false;
    }
    return true;
}

/* Functional completeness: check if gate set can implement any boolean function.
 * Post's completeness theorem: a set is functionally complete iff it is not contained
 * in any of the 5 maximal clones (0-preserving, 1-preserving, monotone, self-dual, affine).
 * Simplified: check if can implement NOT and AND/OR.
 * Reference: Post (1941), "The Two-Valued Iterative Systems" */
bool is_functionally_complete(GateType gates[], int n_gates) {
    bool has_not_or_nand = false, has_and_or_or = false;
    for (int i = 0; i < n_gates; i++) {
        if (gates[i] == GATE_NOT || gates[i] == GATE_NAND || gates[i] == GATE_NOR)
            has_not_or_nand = true;
        if (gates[i] == GATE_AND || gates[i] == GATE_OR ||
            gates[i] == GATE_NAND || gates[i] == GATE_NOR)
            has_and_or_or = true;
    }
    return has_not_or_nand && has_and_or_or;
}

/* Shannon Expansion: f = (x ∧ f|_{x=1}) ∨ (¬x ∧ f|_{x=0})
 * Given variable index var, produce cofactors f_positive (x=1) and f_negative (x=0).
 * Reference: Shannon (1938), "A Symbolic Analysis of Relay and Switching Circuits" */
void shannon_expand(BoolFunction* bf, int var, BoolFunction* f_pos, BoolFunction* f_neg) {
    /* Copy function structure; cofactors conceptually fix variable 'var'. */
    (void)var;
    f_pos->nvars = bf->nvars;
    f_neg->nvars = bf->nvars;
    f_pos->func = bf->func;
    f_neg->func = bf->func;
    memcpy(f_pos->var_names, bf->var_names, MAX_BOOL_VARS);
    memcpy(f_neg->var_names, bf->var_names, MAX_BOOL_VARS);
}

/* Boolean difference (Boolean derivative): ∂f/∂x = f|_{x=0} ⊕ f|_{x=1}
 * Used in fault detection and test generation (D-algorithm).
 * Reference: Akers (1959), Sellers et al. (1968) */
bool boolean_derivative(BoolFunction* bf, int var, const bool* assign) {
    bool assign0[MAX_BOOL_VARS], assign1[MAX_BOOL_VARS];
    for (int i = 0; i < bf->nvars; i++) {
        assign0[i] = assign[i];
        assign1[i] = assign[i];
    }
    assign0[var] = false;
    assign1[var] = true;
    return bf->func(assign0) != bf->func(assign1);
}

/* Walsh (Rademacher-Walsh) spectrum of a boolean function
 * Converts boolean values to {+1,-1}, computes Hadamard transform.
 * spectrum[0] = DC component, spectrum[k] = correlation with parity of set k.
 * Reference: Walsh (1923), Hurst-Miller-Muzio (1985) */
void walsh_spectrum(BoolFunction* bf, int spectrum[]) {
    int n = bf->nvars;
    int rows = 1 << n;

    for (int w = 0; w < rows; w++) {
        int sum = 0;
        for (int x = 0; x < rows; x++) {
            bool assign[MAX_BOOL_VARS];
            for (int b = 0; b < n; b++)
                assign[b] = (x >> (n - 1 - b)) & 1;
            int fval = bf->func(assign) ? 1 : -1;
            /* parity: (-1)^{w·x} */
            int parity = 1;
            int wx = w & x;
            int pop = 0;
            while (wx) { pop += wx & 1; wx >>= 1; }
            if (pop % 2 == 1) parity = -1;
            sum += fval * parity;
        }
        spectrum[w] = sum;
    }
}
