#ifndef SIGNAL_FLOW_H
#define SIGNAL_FLOW_H

#include <stdbool.h>

/* L1: Signal Flow Graph node — represents a variable in the graph */
typedef struct {
    int id;
    double value;
} SFNode;

/* L1: Signal Flow Graph edge — directed branch with transmittance (gain) */
typedef struct {
    int from;
    int to;
    double gain;
} SFEdge;

/* L1: Signal Flow Graph — nodes connected by directed branches */
typedef struct {
    SFNode* nodes;
    SFEdge* edges;
    int n_nodes;
    int n_edges;
    int capacity_nodes;
    int capacity_edges;
} SignalFlowGraph;

/* L1: Forward path — sequence of nodes from source to sink */
typedef struct {
    int* nodes;
    int length;
    double gain;
} ForwardPath;

/* L1: Loop — a closed cycle in the signal flow graph */
typedef struct {
    int* nodes;
    int length;
    double gain;
} SFLoop;

/* L1: Mason's gain formula result */
typedef struct {
    double transfer_function;
    int n_forward_paths;
    int n_loops;
} MasonResult;

/* Construction and destruction */
SignalFlowGraph* sfg_create(void);
void sfg_free(SignalFlowGraph* g);
int sfg_add_node(SignalFlowGraph* g);
void sfg_add_edge(SignalFlowGraph* g, int from, int to, double gain);

/* L5: Depth-first search for forward paths and loops */
int sfg_find_forward_paths(SignalFlowGraph* g, int source, int sink,
                           ForwardPath** paths_out);
int sfg_find_loops(SignalFlowGraph* g, SFLoop** loops_out);
void sfg_free_paths(ForwardPath* paths, int n);
void sfg_free_loops(SFLoop* loops, int n);

/* L3 / L4: Mason's gain formula — computes overall transfer function
 * Mason's theorem: T = (Σ P_k Δ_k) / Δ
 * where Δ = 1 - ΣL_i + ΣL_i L_j - ΣL_i L_j L_k + ... (non-touching loops)
 * Reference: Mason, S.J. (1956) "Feedback Theory"
 */
double sfg_masons_gain(SignalFlowGraph* g, int source, int sink);

/* L2: Block diagram reduction — series, parallel, feedback interconnections */
double block_diagram_series(double g1, double g2);
double block_diagram_parallel(double g1, double g2);
double block_diagram_feedback(double forward, double feedback, bool negative);

/* L7: State-space to transfer function conversion (leverrier algorithm) */
void ss_to_transfer_function(double** A, double** B, double** C, int n,
                              int input_idx, int output_idx,
                              double** num_out, int* num_len,
                              double** den_out, int* den_len);

void sfg_print(SignalFlowGraph* g);

#endif
