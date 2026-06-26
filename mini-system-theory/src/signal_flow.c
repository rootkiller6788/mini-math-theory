/* signal_flow.c - Signal Flow Graphs and Mason's Gain Formula
 *
 * Knowledge coverage:
 * L1: SignalFlowGraph, SFNode, SFEdge, ForwardPath, SFLoop structs
 * L2: Signal flow graph as directed graph with transmittance
 * L3: Graph construction, adjacency representation, DFS traversal
 * L4: Mason's gain formula (Mason, 1956)
 * L5: DFS-based path enumeration, touch-set computation, determinant expansion
 * L6: Transfer function from block diagram via signal flow graph
 * L7: Application to control system analysis (feedback loop reduction)
 * Course alignment: MIT 6.241J Ch. 3 (Block Diagrams), CMU 18-476
 */

#include "signal_flow.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define INIT_CAPACITY 16

SignalFlowGraph* sfg_create(void) {
    SignalFlowGraph* g = (SignalFlowGraph*)malloc(sizeof(SignalFlowGraph));
    g->nodes = (SFNode*)malloc(INIT_CAPACITY * sizeof(SFNode));
    g->edges = (SFEdge*)malloc(INIT_CAPACITY * sizeof(SFEdge));
    g->n_nodes = 0;
    g->n_edges = 0;
    g->capacity_nodes = INIT_CAPACITY;
    g->capacity_edges = INIT_CAPACITY;
    return g;
}

void sfg_free(SignalFlowGraph* g) {
    if (!g) return;
    free(g->nodes);
    free(g->edges);
    free(g);
}

int sfg_add_node(SignalFlowGraph* g) {
    if (g->n_nodes >= g->capacity_nodes) {
        g->capacity_nodes *= 2;
        g->nodes = (SFNode*)realloc(g->nodes, g->capacity_nodes * sizeof(SFNode));
    }
    g->nodes[g->n_nodes].id = g->n_nodes;
    g->nodes[g->n_nodes].value = 0.0;
    return g->n_nodes++;
}

void sfg_add_edge(SignalFlowGraph* g, int from, int to, double gain) {
    if (g->n_edges >= g->capacity_edges) {
        g->capacity_edges *= 2;
        g->edges = (SFEdge*)realloc(g->edges, g->capacity_edges * sizeof(SFEdge));
    }
    g->edges[g->n_edges].from = from;
    g->edges[g->n_edges].to = to;
    g->edges[g->n_edges].gain = gain;
    g->n_edges++;
}

static double** build_adjacency(SignalFlowGraph* g) {
    int n = g->n_nodes;
    double** adj = (double**)malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++) {
        adj[i] = (double*)calloc(n, sizeof(double));
    }
    for (int i = 0; i < g->n_edges; i++) {
        int f = g->edges[i].from;
        int t = g->edges[i].to;
        adj[f][t] = g->edges[i].gain;
    }
    return adj;
}

/* L5: DFS Forward Path Enumeration - finds all simple paths from source to sink */
static void dfs_paths(double** adj, int n, int current, int sink,
                      int* visited, int* path, int depth,
                      double gain, ForwardPath** paths, int* count, int* cap) {
    if (current == sink && depth > 0) {
        if (*count >= *cap) {
            *cap *= 2;
            *paths = (ForwardPath*)realloc(*paths, *cap * sizeof(ForwardPath));
        }
        ForwardPath* fp = &(*paths)[*count];
        fp->nodes = (int*)malloc(depth * sizeof(int));
        memcpy(fp->nodes, path, depth * sizeof(int));
        fp->length = depth;
        fp->gain = gain;
        (*count)++;
        return;
    }
    visited[current] = 1;
    for (int next = 0; next < n; next++) {
        if (fabs(adj[current][next]) > 1e-12 && !visited[next]) {
            path[depth] = next;
            dfs_paths(adj, n, next, sink, visited, path, depth + 1,
                      gain * adj[current][next], paths, count, cap);
        }
    }
    visited[current] = 0;
}

int sfg_find_forward_paths(SignalFlowGraph* g, int source, int sink,
                           ForwardPath** paths_out) {
    double** adj = build_adjacency(g);
    int n = g->n_nodes;
    int* visited = (int*)calloc(n, sizeof(int));
    int* path = (int*)malloc(n * sizeof(int));
    int cap = 16;
    int count = 0;
    *paths_out = (ForwardPath*)malloc(cap * sizeof(ForwardPath));

    visited[source] = 1;
    path[0] = source;
    for (int next = 0; next < n; next++) {
        if (fabs(adj[source][next]) > 1e-12 && !visited[next]) {
            path[1] = next;
            dfs_paths(adj, n, next, sink, visited, path, 2,
                      adj[source][next], paths_out, &count, &cap);
        }
    }

    for (int i = 0; i < n; i++) free(adj[i]);
    free(adj);
    free(visited);
    free(path);
    return count;
}

/* L5: Loop Enumeration - finds all simple cycles in the graph */
static void dfs_loops(double** adj, int n, int start, int current,
                      int* visited, int* path, int depth,
                      double gain, SFLoop** loops, int* count, int* cap) {
    for (int next = 0; next < n; next++) {
        if (fabs(adj[current][next]) > 1e-12) {
            if (next == start && depth >= 1) {
                if (*count >= *cap) {
                    *cap *= 2;
                    *loops = (SFLoop*)realloc(*loops, *cap * sizeof(SFLoop));
                }
                SFLoop* lp = &(*loops)[*count];
                lp->nodes = (int*)malloc((depth + 1) * sizeof(int));
                memcpy(lp->nodes, path, depth * sizeof(int));
                lp->nodes[depth] = start;
                lp->length = depth + 1;
                lp->gain = gain * adj[current][next];
                (*count)++;
            } else if (!visited[next] && next > start) {
                visited[next] = 1;
                path[depth] = next;
                dfs_loops(adj, n, start, next, visited, path, depth + 1,
                          gain * adj[current][next], loops, count, cap);
                visited[next] = 0;
            }
        }
    }
}

int sfg_find_loops(SignalFlowGraph* g, SFLoop** loops_out) {
    double** adj = build_adjacency(g);
    int n = g->n_nodes;
    int* visited = (int*)calloc(n, sizeof(int));
    int* path = (int*)malloc(n * sizeof(int));
    int cap = 16;
    int count = 0;
    *loops_out = (SFLoop*)malloc(cap * sizeof(SFLoop));

    for (int start = 0; start < n; start++) {
        for (int next = 0; next < n; next++) {
            if (fabs(adj[start][next]) > 1e-12) {
                visited[start] = 1;
                visited[next] = 1;
                path[0] = start;
                path[1] = next;
                dfs_loops(adj, n, start, next, visited, path, 2,
                          adj[start][next], loops_out, &count, &cap);
                visited[next] = 0;
                visited[start] = 0;
            }
        }
    }

    for (int i = 0; i < n; i++) free(adj[i]);
    free(adj);
    free(visited);
    free(path);
    return count;
}

void sfg_free_paths(ForwardPath* paths, int n) {
    for (int i = 0; i < n; i++) free(paths[i].nodes);
    free(paths);
}

void sfg_free_loops(SFLoop* loops, int n) {
    for (int i = 0; i < n; i++) free(loops[i].nodes);
    free(loops);
}

/* L4: Mason's Gain Formula (Mason, 1956)
 * T = (Sum P_k * Delta_k) / Delta
 * Delta = 1 - Sum L_i + Sum L_i L_j - ... (non-touching loops, alternating signs)
 */
static bool sets_touch(int* set1, int len1, int* set2, int len2) {
    for (int i = 0; i < len1; i++)
        for (int j = 0; j < len2; j++)
            if (set1[i] == set2[j]) return true;
    return false;
}

static double compute_delta_combinations(SFLoop* loops, int n_loops, int start,
                                         int* current, int depth, int max_depth,
                                         int* touch_set, int touch_len) {
    if (depth == max_depth) {
        double product = 1.0;
        for (int i = 0; i < depth; i++)
            product *= loops[current[i]].gain;
        return product;
    }
    double sum = 0.0;
    for (int i = start; i < n_loops; i++) {
        bool nontouching = true;
        for (int j = 0; j < depth; j++) {
            SFLoop* la = &loops[current[j]];
            SFLoop* lb = &loops[i];
            if (sets_touch(la->nodes, la->length, lb->nodes, lb->length)) {
                nontouching = false;
                break;
            }
        }
        if (!nontouching) continue;
        if (sets_touch(loops[i].nodes, loops[i].length, touch_set, touch_len))
            continue;
        current[depth] = i;
        sum += compute_delta_combinations(loops, n_loops, i + 1, current,
                                           depth + 1, max_depth,
                                           touch_set, touch_len);
    }
    return sum;
}

static double compute_delta(SFLoop* loops, int n_loops,
                            int* exclude_set, int ex_len) {
    double delta = 1.0;
    int* combination = (int*)malloc((n_loops + 1) * sizeof(int));
    int sign = -1;
    for (int k = 1; k <= n_loops; k++) {
        double sum_k = compute_delta_combinations(loops, n_loops, 0, combination,
                                                   0, k, exclude_set, ex_len);
        delta += sign * sum_k;
        sign = -sign;
    }
    free(combination);
    return delta;
}

double sfg_masons_gain(SignalFlowGraph* g, int source, int sink) {
    ForwardPath* paths = NULL;
    SFLoop* loops = NULL;
    int n_paths = sfg_find_forward_paths(g, source, sink, &paths);
    int n_loops = sfg_find_loops(g, &loops);
    double delta = compute_delta(loops, n_loops, NULL, 0);
    if (fabs(delta) < 1e-12) {
        sfg_free_paths(paths, n_paths);
        sfg_free_loops(loops, n_loops);
        return INFINITY;
    }
    double numerator = 0.0;
    for (int k = 0; k < n_paths; k++) {
        double delta_k = compute_delta(loops, n_loops,
                                        paths[k].nodes, paths[k].length);
        numerator += paths[k].gain * delta_k;
    }
    sfg_free_paths(paths, n_paths);
    sfg_free_loops(loops, n_loops);
    return numerator / delta;
}

/* Block Diagram Algebra */
double block_diagram_series(double g1, double g2) { return g1 * g2; }

double block_diagram_parallel(double g1, double g2) { return g1 + g2; }

double block_diagram_feedback(double forward, double feedback, bool negative) {
    if (negative) return forward / (1.0 + forward * feedback);
    else return forward / (1.0 - forward * feedback);
}

/* L7: Leverrier's Algorithm - SS to Transfer Function */
void ss_to_transfer_function(double** A, double** B, double** C, int n,
                              int input_idx, int output_idx,
                              double** num_out, int* num_len,
                              double** den_out, int* den_len) {
    *den_len = n + 1;
    *den_out = (double*)malloc(*den_len * sizeof(double));
    double*** Sk = (double***)malloc((n + 1) * sizeof(double**));
    for (int p = 0; p <= n; p++) {
        Sk[p] = (double**)malloc(n * sizeof(double*));
        for (int i = 0; i < n; i++)
            Sk[p][i] = (double*)calloc(n, sizeof(double));
    }
    for (int i = 0; i < n; i++) Sk[0][i][i] = 1.0;
    (*den_out)[n] = 1.0;
    for (int k = 0; k < n; k++) {
        double tk = 0.0;
        for (int i = 0; i < n; i++) {
            double sum = 0.0;
            for (int j = 0; j < n; j++) sum += A[i][j] * Sk[k][j][i];
            tk += sum;
        }
        double ak = -tk / (k + 1);
        (*den_out)[n - 1 - k] = ak;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                double sum = 0.0;
                for (int p = 0; p < n; p++) sum += A[i][p] * Sk[k][p][j];
                Sk[k + 1][i][j] = sum;
            }
            Sk[k + 1][i][i] += ak;
        }
    }
    *num_len = n;
    *num_out = (double*)calloc(*num_len, sizeof(double));
    for (int k = 0; k < n; k++) {
        double coeff = 0.0;
        for (int i = 0; i < n; i++) {
            double ci = C[output_idx][i];
            if (fabs(ci) < 1e-15) continue;
            double sum = 0.0;
            for (int j = 0; j < n; j++)
                sum += Sk[k][i][j] * B[j][input_idx];
            coeff += ci * sum;
        }
        (*num_out)[n - 1 - k] = coeff;
    }
    for (int p = 0; p <= n; p++) {
        for (int i = 0; i < n; i++) free(Sk[p][i]);
        free(Sk[p]);
    }
    free(Sk);
}

void sfg_print(SignalFlowGraph* g) {
    printf("Signal Flow Graph: %d nodes, %d edges\n", g->n_nodes, g->n_edges);
    printf("Edges:\n");
    for (int i = 0; i < g->n_edges; i++) {
        printf("  %d -> %d [gain=%.4f]\n",
               g->edges[i].from, g->edges[i].to, g->edges[i].gain);
    }
}
