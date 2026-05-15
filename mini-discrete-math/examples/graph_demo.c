#include "graph.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("====== 04 图论 ======\n\n");

    // 1. 创建图
    printf("--- 无向图 BFS/DFS ---\n");
    Graph g;
    graph_init(&g, 6, false);
    graph_add_edge(&g, 0, 1, 1);
    graph_add_edge(&g, 0, 2, 1);
    graph_add_edge(&g, 1, 3, 1);
    graph_add_edge(&g, 1, 4, 1);
    graph_add_edge(&g, 2, 4, 1);
    graph_add_edge(&g, 3, 5, 1);
    graph_add_edge(&g, 4, 5, 1);
    graph_print(&g);

    int order[MAX_VERTICES];
    bfs(&g, 0, order);
    printf("BFS(0): ");
    for (int i = 0; i < g.nvertices; i++) printf("%d ", order[i]);
    printf("\n");

    dfs(&g, 0, order);
    printf("DFS(0): ");
    for (int i = 0; i < g.nvertices; i++) printf("%d ", order[i]);
    printf("\n\n");

    // 2. Dijkstra
    printf("--- 最短路径 (Dijkstra) ---\n");
    int dist[MAX_VERTICES], prev[MAX_VERTICES];
    shortest_path_dijkstra(&g, 0, dist, prev);
    for (int i = 0; i < g.nvertices; i++)
        printf("  0 -> %d: dist=%d, prev=%d\n", i, dist[i], prev[i]);
    printf("\n");

    // 3. 加权图 + Bellman-Ford
    printf("--- Bellman-Ford (含负权) ---\n");
    Graph gw;
    graph_init(&gw, 5, true);
    graph_add_edge(&gw, 0, 1, 6);
    graph_add_edge(&gw, 0, 2, 7);
    graph_add_edge(&gw, 1, 2, 8);
    graph_add_edge(&gw, 1, 3, 5);
    graph_add_edge(&gw, 1, 4, -4);
    graph_add_edge(&gw, 2, 3, -3);
    graph_add_edge(&gw, 2, 4, 9);
    graph_add_edge(&gw, 3, 1, -2);
    graph_add_edge(&gw, 4, 0, 2);
    graph_add_edge(&gw, 4, 3, 7);

    shortest_path_bellman_ford(&gw, 0, dist, prev);
    for (int i = 0; i < gw.nvertices; i++)
        printf("  0 -> %d: dist=%d\n", i, dist[i]);
    printf("\n");

    // 4. 拓扑排序
    printf("--- 拓扑排序 (Kahn) ---\n");
    Graph dag;
    graph_init(&dag, 6, true);
    graph_add_edge(&dag, 5, 2, 1);
    graph_add_edge(&dag, 5, 0, 1);
    graph_add_edge(&dag, 4, 0, 1);
    graph_add_edge(&dag, 4, 1, 1);
    graph_add_edge(&dag, 2, 3, 1);
    graph_add_edge(&dag, 3, 1, 1);

    int* topo = topological_sort_kahn(&dag);
    printf("  拓扑序: ");
    for (int i = 0; i < dag.nvertices; i++) printf("%d ", topo[i]);
    printf("\n");
    printf("  有环? %s\n\n", has_cycle(&dag) ? "是" : "否");
    free(topo);

    // 5. 二分图检测
    printf("--- 二分图检测 ---\n");
    int color[MAX_VERTICES];
    if (is_bipartite(&g, color)) {
        printf("  是二分图\n");
        for (int i = 0; i < g.nvertices; i++)
            printf("    顶点 %d: 颜色 %d\n", i, color[i]);
    }
    printf("\n");

    // 6. 图着色
    printf("--- 贪心图着色 ---\n");
    int cols[MAX_VERTICES];
    int ncolors = graph_coloring_greedy(&g, cols);
    printf("  用色数: %d\n", ncolors);
    for (int i = 0; i < g.nvertices; i++)
        printf("    顶点 %d: 颜色 %d\n", i, cols[i]);
    printf("\n");

    // 7. 欧拉回路
    printf("--- 欧拉回路 ---\n");
    Graph euler_g;
    graph_init(&euler_g, 5, false);
    graph_add_edge(&euler_g, 0, 1, 1);
    graph_add_edge(&euler_g, 0, 3, 1);
    graph_add_edge(&euler_g, 1, 2, 1);
    graph_add_edge(&euler_g, 1, 4, 1);
    graph_add_edge(&euler_g, 2, 3, 1);
    graph_add_edge(&euler_g, 3, 4, 1);

    printf("  有欧拉回路? %s\n", has_euler_circuit(&euler_g) ? "是" : "否");
    int path[MAX_VERTICES * MAX_VERTICES];
    int path_len;
    find_euler_circuit(&euler_g, path, &path_len);
    if (path_len > 0) {
        printf("  欧拉回路: ");
        for (int i = 0; i < path_len; i++) printf("%d ", path[i]);
        printf("\n");
    }

    return 0;
}
