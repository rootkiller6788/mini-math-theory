#ifndef GRAPH_ALGOS_H
#define GRAPH_ALGOS_H

#include <stdbool.h>

#define INF 999999
#define MAX_V 256

int** graph_create_adj_matrix(int n);
void graph_free_adj_matrix(int** g, int n);
void graph_add_edge(int** g, int from, int to, int weight);

void bfs_matrix(int** g, int n, int start, int* order);
void dfs_matrix(int** g, int n, int start, int* order);
void dijkstra(int** g, int n, int start, int* dist, int* prev);
void bellman_ford(int** g, int n, int start, int* dist, int* prev);
void floyd_warshall(int** g, int n, int** dist);
void prim_mst(int** g, int n, int* parent);
void kruskal_mst(int n, int edges[][3], int nedges, int mst[][3]);
int topological_sort(int** g, int n, int* order);
void kosaraju_scc(int** g, int n, int* component);
bool has_cycle_directed(int** g, int n);
bool has_cycle_undirected(int** g, int n);
int bfs_shortest_path(int** g, int n, int from, int to);

#endif
