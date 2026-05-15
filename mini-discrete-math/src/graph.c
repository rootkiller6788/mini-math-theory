#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

void graph_init(Graph* g, int n, bool directed) {
    g->nvertices = n;
    g->nedges = 0;
    g->directed = directed;
    for (int i = 0; i < n; i++)
        g->adj[i] = NULL;
}

void graph_add_edge(Graph* g, int from, int to, int weight) {
    EdgeNode* node = malloc(sizeof(EdgeNode));
    node->to = to;
    node->weight = weight;
    node->next = g->adj[from];
    g->adj[from] = node;
    g->nedges++;

    if (!g->directed) {
        EdgeNode* rev = malloc(sizeof(EdgeNode));
        rev->to = from;
        rev->weight = weight;
        rev->next = g->adj[to];
        g->adj[to] = rev;
    }
}

void graph_print(Graph* g) {
    printf("图: %d 顶点, %d 边 (%s)\n",
           g->nvertices, g->nedges, g->directed ? "有向" : "无向");
    for (int i = 0; i < g->nvertices; i++) {
        printf("  %d -> ", i);
        for (EdgeNode* e = g->adj[i]; e; e = e->next)
            printf("%d(w=%d) ", e->to, e->weight);
        printf("\n");
    }
}

void bfs(Graph* g, int start, int* order) {
    bool visited[MAX_VERTICES] = {false};
    int queue[MAX_VERTICES];
    int head = 0, tail = 0;
    int idx = 0;

    visited[start] = true;
    queue[tail++] = start;

    while (head < tail) {
        int v = queue[head++];
        order[idx++] = v;
        for (EdgeNode* e = g->adj[v]; e; e = e->next) {
            if (!visited[e->to]) {
                visited[e->to] = true;
                queue[tail++] = e->to;
            }
        }
    }
}

static void dfs_rec(Graph* g, int v, bool* visited, int* order, int* idx) {
    visited[v] = true;
    order[(*idx)++] = v;
    for (EdgeNode* e = g->adj[v]; e; e = e->next)
        if (!visited[e->to])
            dfs_rec(g, e->to, visited, order, idx);
}

void dfs(Graph* g, int start, int* order) {
    bool visited[MAX_VERTICES] = {false};
    int idx = 0;
    dfs_rec(g, start, visited, order, &idx);
}

bool has_path(Graph* g, int from, int to) {
    int order[MAX_VERTICES];
    bfs(g, from, order);
    for (int i = 0; i < g->nvertices; i++)
        if (order[i] == to) return true;
    return false;
}

void shortest_path_dijkstra(Graph* g, int start, int* dist, int* prev) {
    bool visited[MAX_VERTICES] = {false};

    for (int i = 0; i < g->nvertices; i++) {
        dist[i] = INF;
        prev[i] = -1;
    }
    dist[start] = 0;

    for (int count = 0; count < g->nvertices; count++) {
        int u = -1, mindist = INF;
        for (int i = 0; i < g->nvertices; i++)
            if (!visited[i] && dist[i] < mindist) {
                mindist = dist[i];
                u = i;
            }
        if (u == -1) break;
        visited[u] = true;

        for (EdgeNode* e = g->adj[u]; e; e = e->next) {
            if (!visited[e->to] && dist[u] + e->weight < dist[e->to]) {
                dist[e->to] = dist[u] + e->weight;
                prev[e->to] = u;
            }
        }
    }
}

void shortest_path_bellman_ford(Graph* g, int start, int* dist, int* prev) {
    for (int i = 0; i < g->nvertices; i++) {
        dist[i] = INF;
        prev[i] = -1;
    }
    dist[start] = 0;

    for (int k = 0; k < g->nvertices - 1; k++) {
        for (int u = 0; u < g->nvertices; u++) {
            for (EdgeNode* e = g->adj[u]; e; e = e->next) {
                if (dist[u] != INF && dist[u] + e->weight < dist[e->to]) {
                    dist[e->to] = dist[u] + e->weight;
                    prev[e->to] = u;
                }
            }
        }
    }
}

void shortest_path_floyd(Graph* g, int dist[MAX_VERTICES][MAX_VERTICES]) {
    for (int i = 0; i < g->nvertices; i++) {
        for (int j = 0; j < g->nvertices; j++)
            dist[i][j] = (i == j) ? 0 : INF;
        for (EdgeNode* e = g->adj[i]; e; e = e->next)
            dist[i][e->to] = e->weight;
    }

    for (int k = 0; k < g->nvertices; k++)
        for (int i = 0; i < g->nvertices; i++)
            for (int j = 0; j < g->nvertices; j++)
                if (dist[i][k] != INF && dist[k][j] != INF &&
                    dist[i][k] + dist[k][j] < dist[i][j])
                    dist[i][j] = dist[i][k] + dist[k][j];
}

int* topological_sort_kahn(Graph* g) {
    int indegree[MAX_VERTICES] = {0};
    int* order = malloc(g->nvertices * sizeof(int));
    int idx = 0;

    for (int u = 0; u < g->nvertices; u++)
        for (EdgeNode* e = g->adj[u]; e; e = e->next)
            indegree[e->to]++;

    int queue[MAX_VERTICES], head = 0, tail = 0;
    for (int i = 0; i < g->nvertices; i++)
        if (indegree[i] == 0)
            queue[tail++] = i;

    while (head < tail) {
        int u = queue[head++];
        order[idx++] = u;
        for (EdgeNode* e = g->adj[u]; e; e = e->next) {
            indegree[e->to]--;
            if (indegree[e->to] == 0)
                queue[tail++] = e->to;
        }
    }
    return order;
}

static bool dfs_has_cycle(Graph* g, int v, int* state) {
    state[v] = 1; // visiting
    for (EdgeNode* e = g->adj[v]; e; e = e->next) {
        if (state[e->to] == 1) return true;
        if (state[e->to] == 0 && dfs_has_cycle(g, e->to, state))
            return true;
    }
    state[v] = 2; // visited
    return false;
}

bool has_cycle(Graph* g) {
    int state[MAX_VERTICES] = {0};
    for (int i = 0; i < g->nvertices; i++)
        if (state[i] == 0 && dfs_has_cycle(g, i, state))
            return true;
    return false;
}

bool is_bipartite(Graph* g, int color[MAX_VERTICES]) {
    for (int i = 0; i < g->nvertices; i++) color[i] = -1;
    for (int start = 0; start < g->nvertices; start++) {
        if (color[start] != -1) continue;
        int queue[MAX_VERTICES], head = 0, tail = 0;
        color[start] = 0;
        queue[tail++] = start;
        while (head < tail) {
            int u = queue[head++];
            for (EdgeNode* e = g->adj[u]; e; e = e->next) {
                if (color[e->to] == -1) {
                    color[e->to] = 1 - color[u];
                    queue[tail++] = e->to;
                } else if (color[e->to] == color[u])
                    return false;
            }
        }
    }
    return true;
}

int graph_coloring_greedy(Graph* g, int result[MAX_VERTICES]) {
    for (int i = 0; i < g->nvertices; i++) result[i] = -1;
    bool used[MAX_VERTICES];
    int max_color = 0;

    for (int u = 0; u < g->nvertices; u++) {
        for (int i = 0; i < MAX_VERTICES; i++) used[i] = false;

        for (EdgeNode* e = g->adj[u]; e; e = e->next)
            if (result[e->to] != -1)
                used[result[e->to]] = true;

        int c;
        for (c = 0; used[c]; c++)
            ;
        result[u] = c;
        if (c + 1 > max_color) max_color = c + 1;
    }
    return max_color;
}

bool has_euler_circuit(Graph* g) {
    for (int i = 0; i < g->nvertices; i++) {
        int degree = 0;
        for (EdgeNode* e = g->adj[i]; e; e = e->next)
            degree++;
        if (degree % 2 != 0) return false;
    }
    return true;
}

void find_euler_circuit(Graph* g, int* path, int* path_len) {
    if (!has_euler_circuit(g)) {
        *path_len = 0;
        return;
    }
    int stack[MAX_VERTICES * MAX_VERTICES], top = 0;
    int* deg = malloc(g->nvertices * sizeof(int));
    int** edges_used = malloc(g->nvertices * sizeof(int*));

    for (int i = 0; i < g->nvertices; i++) {
        deg[i] = 0;
        edges_used[i] = calloc(MAX_VERTICES, sizeof(int));
    }

    stack[top++] = 0;
    *path_len = 0;

    while (top > 0) {
        int v = stack[top - 1];
        int next = -1;
        for (EdgeNode* e = g->adj[v]; e; e = e->next) {
            if (!edges_used[v][e->to]) {
                next = e->to;
                break;
            }
        }
        if (next != -1) {
            edges_used[v][next] = 1;
            if (!g->directed) edges_used[next][v] = 1;
            stack[top++] = next;
        } else {
            path[(*path_len)++] = v;
            top--;
        }
    }

    for (int i = 0; i < g->nvertices; i++) free(edges_used[i]);
    free(edges_used);
    free(deg);
}
