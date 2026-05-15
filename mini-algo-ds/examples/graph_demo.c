#include <stdio.h>
#include <stdlib.h>
#include "graph_algos.h"

void add_undirected(int** g, int u, int v, int w) {
    graph_add_edge(g, u, v, w);
    graph_add_edge(g, v, u, w);
}

int main(void) {
    printf("=== Graph Algorithms Demo ===\n\n");

    int V = 9;
    int** g = graph_create_adj_matrix(V);

    // Undirected graph based on CLRS Fig 23.1
    add_undirected(g, 0, 1, 4);
    add_undirected(g, 0, 7, 8);
    add_undirected(g, 1, 2, 8);
    add_undirected(g, 1, 7, 11);
    add_undirected(g, 2, 3, 7);
    add_undirected(g, 2, 5, 4);
    add_undirected(g, 2, 8, 2);
    add_undirected(g, 3, 4, 9);
    add_undirected(g, 3, 5, 14);
    add_undirected(g, 4, 5, 10);
    add_undirected(g, 5, 6, 2);
    add_undirected(g, 6, 7, 1);
    add_undirected(g, 6, 8, 6);
    add_undirected(g, 7, 8, 7);

    printf("--- 1. BFS from 0 ---\n");
    int bfs_order[V];
    bfs_matrix(g, V, 0, bfs_order);
    printf("BFS: ");
    for (int i = 0; i < V; i++) printf("%d ", bfs_order[i]);
    printf("\n\n");

    printf("--- 2. DFS from 0 ---\n");
    int dfs_order[V];
    dfs_matrix(g, V, 0, dfs_order);
    printf("DFS: ");
    for (int i = 0; i < V; i++) printf("%d ", dfs_order[i]);
    printf("\n\n");

    printf("--- 3. Dijkstra from 0 ---\n");
    int dist[V], prev[V];
    dijkstra(g, V, 0, dist, prev);
    for (int i = 0; i < V; i++)
        printf("  dist[%d] = %d\n", i, dist[i]);
    printf("\n");

    printf("--- 4. Prim's MST ---\n");
    int mst_parent[V];
    prim_mst(g, V, mst_parent);
    int total_w = 0;
    printf("MST edges:\n");
    for (int i = 1; i < V; i++) {
        int p = mst_parent[i];
        total_w += g[p][i];
        printf("  %d -- %d (weight=%d)\n", p, i, g[p][i]);
    }
    printf("Total weight: %d\n\n", total_w);

    printf("--- 5. Topological Sort (DAG) ---\n");
    int dag_n = 6;
    int** dag = graph_create_adj_matrix(dag_n);
    graph_add_edge(dag, 5, 2, 1);
    graph_add_edge(dag, 5, 0, 1);
    graph_add_edge(dag, 4, 0, 1);
    graph_add_edge(dag, 4, 1, 1);
    graph_add_edge(dag, 2, 3, 1);
    graph_add_edge(dag, 3, 1, 1);

    int topo[6];
    int topo_n = topological_sort(dag, dag_n, topo);
    printf("Topo order: ");
    for (int i = 0; i < topo_n; i++) printf("%d ", topo[i]);
    printf("\n\n");
    graph_free_adj_matrix(dag, dag_n);

    printf("--- 6. Kosaraju SCC ---\n");
    int scc_n = 8;
    int** scc_g = graph_create_adj_matrix(scc_n);
    graph_add_edge(scc_g, 0, 1, 1);
    graph_add_edge(scc_g, 1, 2, 1);
    graph_add_edge(scc_g, 2, 0, 1);
    graph_add_edge(scc_g, 2, 3, 1);
    graph_add_edge(scc_g, 3, 4, 1);
    graph_add_edge(scc_g, 4, 5, 1);
    graph_add_edge(scc_g, 5, 3, 1);
    graph_add_edge(scc_g, 6, 5, 1);
    graph_add_edge(scc_g, 6, 7, 1);

    int comp[8];
    kosaraju_scc(scc_g, scc_n, comp);
    printf("Components: ");
    for (int i = 0; i < scc_n; i++) printf("%d ", comp[i]);
    printf("\n  (2 SCCs expected: {0,1,2} and {3,4,5} and {6} and {7})\n\n");
    graph_free_adj_matrix(scc_g, scc_n);

    printf("--- 7. Cycle Detection ---\n");
    int** cyc_d = graph_create_adj_matrix(4);
    graph_add_edge(cyc_d, 0, 1, 1);
    graph_add_edge(cyc_d, 1, 2, 1);
    graph_add_edge(cyc_d, 2, 0, 1);
    graph_add_edge(cyc_d, 2, 3, 1);
    printf("Directed cycle? %s\n", has_cycle_directed(cyc_d, 4) ? "YES" : "NO");
    graph_free_adj_matrix(cyc_d, 4);

    int** cyc_u = graph_create_adj_matrix(4);
    add_undirected(cyc_u, 0, 1, 1);
    add_undirected(cyc_u, 1, 2, 1);
    add_undirected(cyc_u, 2, 3, 1);
    add_undirected(cyc_u, 3, 0, 1);
    printf("Undirected cycle? %s\n", has_cycle_undirected(cyc_u, 4) ? "YES" : "NO");
    graph_free_adj_matrix(cyc_u, 4);

    graph_free_adj_matrix(g, V);
    return 0;
}
