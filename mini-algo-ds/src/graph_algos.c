#include "graph_algos.h"
#include "union_find.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int** graph_create_adj_matrix(int n) {
    int** g = malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        g[i] = malloc(n * sizeof(int));
        for (int j = 0; j < n; j++)
            g[i][j] = (i == j) ? 0 : INF;
    }
    return g;
}

void graph_free_adj_matrix(int** g, int n) {
    for (int i = 0; i < n; i++) free(g[i]);
    free(g);
}

void graph_add_edge(int** g, int from, int to, int weight) {
    g[from][to] = weight;
}

void bfs_matrix(int** g, int n, int start, int* order) {
    bool visited[MAX_V] = {false};
    int queue[MAX_V], head = 0, tail = 0, idx = 0;
    visited[start] = true;
    queue[tail++] = start;
    while (head < tail) {
        int v = queue[head++];
        order[idx++] = v;
        for (int i = 0; i < n; i++)
            if (g[v][i] != INF && g[v][i] != 0 && !visited[i]) {
                visited[i] = true;
                queue[tail++] = i;
            }
    }
}

static void dfs_rec(int** g, int n, int v, bool* visited, int* order, int* idx) {
    visited[v] = true;
    order[(*idx)++] = v;
    for (int i = 0; i < n; i++)
        if (g[v][i] != INF && g[v][i] != 0 && !visited[i])
            dfs_rec(g, n, i, visited, order, idx);
}

void dfs_matrix(int** g, int n, int start, int* order) {
    bool visited[MAX_V] = {false};
    int idx = 0;
    dfs_rec(g, n, start, visited, order, &idx);
}

void dijkstra(int** g, int n, int start, int* dist, int* prev) {
    bool visited[MAX_V] = {false};
    for (int i = 0; i < n; i++) { dist[i] = INF; prev[i] = -1; }
    dist[start] = 0;
    for (int count = 0; count < n; count++) {
        int u = -1, min = INF;
        for (int i = 0; i < n; i++)
            if (!visited[i] && dist[i] < min) { min = dist[i]; u = i; }
        if (u == -1) break;
        visited[u] = true;
        for (int v = 0; v < n; v++)
            if (!visited[v] && g[u][v] != INF && dist[u] + g[u][v] < dist[v]) {
                dist[v] = dist[u] + g[u][v];
                prev[v] = u;
            }
    }
}

void bellman_ford(int** g, int n, int start, int* dist, int* prev) {
    for (int i = 0; i < n; i++) { dist[i] = INF; prev[i] = -1; }
    dist[start] = 0;
    for (int k = 0; k < n - 1; k++)
        for (int u = 0; u < n; u++)
            for (int v = 0; v < n; v++)
                if (g[u][v] != INF && dist[u] != INF && dist[u] + g[u][v] < dist[v]) {
                    dist[v] = dist[u] + g[u][v];
                    prev[v] = u;
                }
}

void floyd_warshall(int** g, int n, int** dist) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            dist[i][j] = g[i][j];
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (dist[i][k] != INF && dist[k][j] != INF &&
                    dist[i][k] + dist[k][j] < dist[i][j])
                    dist[i][j] = dist[i][k] + dist[k][j];
}

void prim_mst(int** g, int n, int* parent) {
    int key[MAX_V];
    bool in_mst[MAX_V] = {false};
    for (int i = 0; i < n; i++) { key[i] = INF; parent[i] = -1; }
    key[0] = 0;
    for (int count = 0; count < n; count++) {
        int u = -1, min = INF;
        for (int i = 0; i < n; i++)
            if (!in_mst[i] && key[i] < min) { min = key[i]; u = i; }
        if (u == -1) break;
        in_mst[u] = true;
        for (int v = 0; v < n; v++)
            if (g[u][v] != INF && !in_mst[v] && g[u][v] < key[v]) {
                key[v] = g[u][v];
                parent[v] = u;
            }
    }
}

static int edge_cmp(const void* a, const void* b) {
    return ((int*)a)[2] - ((int*)b)[2];
}

void kruskal_mst(int n, int edges[][3], int nedges, int mst[][3]) {
    qsort(edges, nedges, 3 * sizeof(int), edge_cmp);
    UnionFind* uf = uf_create(n);
    int mst_size = 0;
    for (int i = 0; i < nedges && mst_size < n - 1; i++) {
        int u = edges[i][0], v = edges[i][1], w = edges[i][2];
        if (!uf_connected(uf, u, v)) {
            uf_union(uf, u, v);
            mst[mst_size][0] = u;
            mst[mst_size][1] = v;
            mst[mst_size][2] = w;
            mst_size++;
        }
    }
    uf_free(uf);
}

int topological_sort(int** g, int n, int* order) {
    int indegree[MAX_V] = {0};
    for (int u = 0; u < n; u++)
        for (int v = 0; v < n; v++)
            if (g[u][v] != INF && g[u][v] != 0) indegree[v]++;
    int queue[MAX_V], head = 0, tail = 0, idx = 0;
    for (int i = 0; i < n; i++)
        if (indegree[i] == 0) queue[tail++] = i;
    while (head < tail) {
        int u = queue[head++];
        order[idx++] = u;
        for (int v = 0; v < n; v++)
            if (g[u][v] != INF && g[u][v] != 0) {
                indegree[v]--;
                if (indegree[v] == 0) queue[tail++] = v;
            }
    }
    return idx;
}

static void dfs_fill_order(int** g, int n, int v, bool* visited, int* stack, int* top) {
    visited[v] = true;
    for (int i = 0; i < n; i++)
        if (g[v][i] != INF && g[v][i] != 0 && !visited[i])
            dfs_fill_order(g, n, i, visited, stack, top);
    stack[(*top)++] = v;
}

static void dfs_collect(int** g, int n, int v, bool* visited, int* comp, int comp_id) {
    visited[v] = true;
    comp[v] = comp_id;
    for (int i = 0; i < n; i++)
        if (g[v][i] != INF && g[v][i] != 0 && !visited[i])
            dfs_collect(g, n, i, visited, comp, comp_id);
}

void kosaraju_scc(int** g, int n, int* component) {
    bool visited[MAX_V] = {false};
    int stack[MAX_V], top = 0;

    for (int i = 0; i < n; i++)
        if (!visited[i])
            dfs_fill_order(g, n, i, visited, stack, &top);

    int** gt = graph_create_adj_matrix(n);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (g[i][j] != INF && g[i][j] != 0)
                gt[j][i] = g[i][j];

    memset(visited, 0, sizeof(bool) * MAX_V);
    int comp_id = 0;
    for (int i = top - 1; i >= 0; i--) {
        int v = stack[i];
        if (!visited[v])
            dfs_collect(gt, n, v, visited, component, comp_id++);
    }
    graph_free_adj_matrix(gt, n);
}

static bool dfs_cycle_directed(int** g, int n, int v, int* state) {
    state[v] = 1;
    for (int i = 0; i < n; i++) {
        if (g[v][i] == INF || g[v][i] == 0) continue;
        if (state[i] == 1) return true;
        if (state[i] == 0 && dfs_cycle_directed(g, n, i, state)) return true;
    }
    state[v] = 2;
    return false;
}

bool has_cycle_directed(int** g, int n) {
    int state[MAX_V] = {0};
    for (int i = 0; i < n; i++)
        if (state[i] == 0 && dfs_cycle_directed(g, n, i, state))
            return true;
    return false;
}

static bool dfs_cycle_undirected(int** g, int n, int v, int parent, bool* visited) {
    visited[v] = true;
    for (int i = 0; i < n; i++) {
        if (g[v][i] == INF || g[v][i] == 0) continue;
        if (!visited[i]) {
            if (dfs_cycle_undirected(g, n, i, v, visited)) return true;
        } else if (i != parent) {
            return true;
        }
    }
    return false;
}

bool has_cycle_undirected(int** g, int n) {
    bool visited[MAX_V] = {false};
    for (int i = 0; i < n; i++)
        if (!visited[i] && dfs_cycle_undirected(g, n, i, -1, visited))
            return true;
    return false;
}

int bfs_shortest_path(int** g, int n, int from, int to) {
    int dist[MAX_V];
    int queue[MAX_V], head = 0, tail = 0;
    bool visited[MAX_V] = {false};
    for (int i = 0; i < n; i++) dist[i] = -1;
    visited[from] = true;
    dist[from] = 0;
    queue[tail++] = from;
    while (head < tail) {
        int v = queue[head++];
        if (v == to) return dist[v];
        for (int i = 0; i < n; i++)
            if (g[v][i] != INF && g[v][i] != 0 && !visited[i]) {
                visited[i] = true;
                dist[i] = dist[v] + 1;
                queue[tail++] = i;
            }
    }
    return -1;
}
