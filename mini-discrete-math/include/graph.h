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

#endif
