#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>

#define MAX_VERTICES 64
#define INF 999999

typedef struct EdgeNode {
    int to;
    int weight;
    struct EdgeNode* next;
} EdgeNode;

typedef EdgeNode* AdjList[MAX_VERTICES];

typedef struct {
    AdjList adj;
    int nvertices;
    int nedges;
    bool directed;
} Graph;

void graph_init(Graph* g, int n, bool directed);
void graph_add_edge(Graph* g, int from, int to, int weight);
void graph_print(Graph* g);

void bfs(Graph* g, int start, int* order);
void dfs(Graph* g, int start, int* order);
void dfs_recursive(Graph* g, int v, bool* visited, int* order, int* idx);

bool has_path(Graph* g, int from, int to);
void shortest_path_dijkstra(Graph* g, int start, int* dist, int* prev);
void shortest_path_bellman_ford(Graph* g, int start, int* dist, int* prev);
void shortest_path_floyd(Graph* g, int dist[MAX_VERTICES][MAX_VERTICES]);

int* topological_sort_kahn(Graph* g);
bool has_cycle(Graph* g);

bool is_bipartite(Graph* g, int color[MAX_VERTICES]);
int graph_coloring_greedy(Graph* g, int color[MAX_VERTICES]);

bool has_euler_circuit(Graph* g);
void find_euler_circuit(Graph* g, int* path, int* path_len);

/* Minimum Spanning Tree (Kruskal's greedy + Union-Find, Prim's) */
void kruskal_mst(Graph* g, int edges_out[MAX_VERTICES][2], int* nedges_out);
int prim_mst(Graph* g, int parent[MAX_VERTICES]);

/* Strongly Connected Components (Kosaraju's two-pass, Tarjan's one-pass) */
void kosaraju_scc(Graph* g, int component[MAX_VERTICES], int* ncomp);
void tarjan_scc(Graph* g, int component[MAX_VERTICES], int* ncomp);

/* Articulation Points (Tarjan-like DFS low-link) */
void articulation_points(Graph* g, bool is_ap[MAX_VERTICES]);

/* Bridges in undirected graph */
void bridges_find(Graph* g, int bridges_out[MAX_VERTICES][2], int* nbridges);

/* Max Flow (Ford-Fulkerson with DFS augmenting paths) */
int ford_fulkerson_maxflow(int capacity[MAX_VERTICES][MAX_VERTICES], int n,
                           int source, int sink);

/* Hamiltonian Cycle (backtracking search) */
bool hamiltonian_cycle(Graph* g, int path[MAX_VERTICES]);

/* Travelling Salesman approximation (MST-based 2-approximation) */
int tsp_mst_2approx(Graph* g, int tour[MAX_VERTICES]);

/* Graph diameter (all-pairs shortest path based) */
int graph_diameter(Graph* g);

/* Maximum matching in bipartite graph (DFS augmenting path) */
int bipartite_max_matching(Graph* g, int match[MAX_VERTICES]);

/* Planarity: Kuratowski-based check for K5/K3,3 minors (simple heuristic) */
bool is_planar_heuristic(Graph* g);

/* Transitive reduction (minimal graph with same reachability) */
void transitive_reduction(Graph* g, Graph* result);

#endif
