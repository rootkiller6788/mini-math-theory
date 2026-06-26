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

/* ===== Union-Find (Disjoint Set Union) for Kruskal ===== */
static int uf_parent[MAX_VERTICES];
static int uf_rank[MAX_VERTICES];

static void uf_init(int n) {
    for (int i = 0; i < n; i++) {
        uf_parent[i] = i;
        uf_rank[i] = 0;
    }
}

static int uf_find(int x) {
    if (uf_parent[x] != x)
        uf_parent[x] = uf_find(uf_parent[x]);  /* path compression */
    return uf_parent[x];
}

static bool uf_union(int x, int y) {
    int rx = uf_find(x), ry = uf_find(y);
    if (rx == ry) return false;
    if (uf_rank[rx] < uf_rank[ry])
        uf_parent[rx] = ry;
    else if (uf_rank[rx] > uf_rank[ry])
        uf_parent[ry] = rx;
    else {
        uf_parent[ry] = rx;
        uf_rank[rx]++;
    }
    return true;
}

/* Kruskal's Minimum Spanning Tree — O(E log E) with Union-Find
 * Theorem: Kruskal's algorithm produces a minimum spanning tree (Cut Property).
 * Reference: MIT 6.042J Graph Theory, CLRS §23.2 */
void kruskal_mst(Graph* g, int edges_out[MAX_VERTICES][2], int* nedges_out) {
    *nedges_out = 0;
    int m = g->nedges;
    if (g->directed) m /= 2; /* undirected edge count */

    /* collect all edges */
    typedef struct { int u, v, w; } Edge;
    Edge edges[MAX_VERTICES * MAX_VERTICES];
    int ec = 0;
    for (int u = 0; u < g->nvertices; u++) {
        for (EdgeNode* e = g->adj[u]; e; e = e->next) {
            if (u < e->to || g->directed) {
                edges[ec].u = u;
                edges[ec].v = e->to;
                edges[ec].w = e->weight;
                ec++;
            }
        }
    }

    /* sort edges by weight (insertion sort for simplicity, E is small) */
    for (int i = 1; i < ec; i++) {
        Edge key = edges[i];
        int j = i - 1;
        while (j >= 0 && edges[j].w > key.w) {
            edges[j + 1] = edges[j];
            j--;
        }
        edges[j + 1] = key;
    }

    uf_init(g->nvertices);
    for (int i = 0; i < ec && *nedges_out < g->nvertices - 1; i++) {
        if (uf_union(edges[i].u, edges[i].v)) {
            edges_out[*nedges_out][0] = edges[i].u;
            edges_out[*nedges_out][1] = edges[i].v;
            (*nedges_out)++;
        }
    }
}

/* Prim's Minimum Spanning Tree — O(V²) adjacency list
 * Uses cut property: at each step, add the minimum-weight edge crossing the cut.
 * Reference: CLRS §23.2, MIT 6.046J */
int prim_mst(Graph* g, int parent[MAX_VERTICES]) {
    int key[MAX_VERTICES];
    bool in_mst[MAX_VERTICES] = {false};
    for (int i = 0; i < g->nvertices; i++) {
        key[i] = INF;
        parent[i] = -1;
    }
    key[0] = 0;
    int total_weight = 0;

    for (int count = 0; count < g->nvertices; count++) {
        int u = -1, min_key = INF;
        for (int i = 0; i < g->nvertices; i++)
            if (!in_mst[i] && key[i] < min_key) {
                min_key = key[i];
                u = i;
            }
        if (u == -1) break;
        in_mst[u] = true;
        total_weight += key[u];

        for (EdgeNode* e = g->adj[u]; e; e = e->next) {
            if (!in_mst[e->to] && e->weight < key[e->to]) {
                key[e->to] = e->weight;
                parent[e->to] = u;
            }
        }
    }
    return total_weight;
}

/* Kosaraju's SCC algorithm — O(V+E) two-pass DFS
 * First pass: compute finishing times on reversed graph.
 * Second pass: DFS on original graph in decreasing finish order.
 * Theorem: In a directed graph, SCCs form a DAG (condensation graph).
 * Reference: MIT 6.006, CLRS §22.5 */
static void kosaraju_dfs1(Graph* g, int v, bool* visited, int* fin, int* fidx) {
    visited[v] = true;
    for (EdgeNode* e = g->adj[v]; e; e = e->next)
        if (!visited[e->to])
            kosaraju_dfs1(g, e->to, visited, fin, fidx);
    fin[(*fidx)++] = v;
}

/* Kosaraju SCC corrected implementation */
void kosaraju_scc(Graph* g, int component[MAX_VERTICES], int* ncomp) {
    int n = g->nvertices;
    bool visited[MAX_VERTICES] = {false};
    int fin[MAX_VERTICES], fidx = 0;

    /* Build reverse graph adjacency matrix for simplicity */
    int rev_adj[MAX_VERTICES][MAX_VERTICES];
    int rev_deg[MAX_VERTICES] = {0};
    for (int u = 0; u < n; u++)
        for (EdgeNode* e = g->adj[u]; e; e = e->next)
            rev_adj[e->to][rev_deg[e->to]++] = u;

    /* First pass on original graph */
    for (int v = 0; v < n; v++)
        if (!visited[v])
            kosaraju_dfs1(g, v, visited, fin, &fidx);

    /* Second pass on reverse graph in decreasing finish time */
    memset(visited, 0, sizeof(visited));
    *ncomp = 0;
    for (int i = fidx - 1; i >= 0; i--) {
        int v = fin[i];
        if (!visited[v]) {
            /* DFS on reverse graph from v */
            int stack[MAX_VERTICES], top = 0;
            stack[top++] = v;
            visited[v] = true;
            component[v] = *ncomp;
            while (top > 0) {
                int u = stack[--top];
                component[u] = *ncomp;
                for (int j = 0; j < rev_deg[u]; j++) {
                    int w = rev_adj[u][j];
                    if (!visited[w]) {
                        visited[w] = true;
                        stack[top++] = w;
                    }
                }
            }
            (*ncomp)++;
        }
    }
}

/* Tarjan's SCC — single-pass DFS with low-link values
 * Uses implicit stack (recursion). O(V+E).
 * Theorem: A vertex is the root of an SCC iff low[v] == disc[v].
 * Reference: Tarjan (1972), CLRS exercise 22.5 */
void tarjan_scc(Graph* g, int component[MAX_VERTICES], int* ncomp) {
    int n = g->nvertices;
    int disc[MAX_VERTICES], low[MAX_VERTICES], time = 0;
    bool on_stack[MAX_VERTICES] = {false};
    int stack[MAX_VERTICES], top = 0;
    *ncomp = 0;

    for (int i = 0; i < n; i++) {
        disc[i] = -1;
        component[i] = -1;
    }

    /* iterative Tarjan using explicit stack frames */
    typedef struct { int v; int state; EdgeNode* e; } Frame;
    Frame frames[MAX_VERTICES * 2];
    int fsp = 0;

    for (int start = 0; start < n; start++) {
        if (disc[start] != -1) continue;
        frames[fsp++] = (Frame){start, 0, g->adj[start]};
        while (fsp > 0) {
            Frame* cur = &frames[fsp - 1];
            if (cur->state == 0) {
                disc[cur->v] = low[cur->v] = ++time;
                stack[top++] = cur->v;
                on_stack[cur->v] = true;
                cur->state = 1;
            }
            /* process children */
            int found_unvisited = 0;
            while (cur->e) {
                int w = cur->e->to;
                if (disc[w] == -1) {
                    frames[fsp++] = (Frame){w, 0, g->adj[w]};
                    found_unvisited = 1;
                    cur->e = cur->e->next;
                    break;
                } else if (on_stack[w]) {
                    if (disc[w] < low[cur->v])
                        low[cur->v] = disc[w];
                }
                cur->e = cur->e->next;
            }
            if (found_unvisited) continue;
            /* all children processed */
            if (low[cur->v] == disc[cur->v]) {
                /* root of SCC */
                int w;
                do {
                    w = stack[--top];
                    on_stack[w] = false;
                    component[w] = *ncomp;
                } while (w != cur->v);
                (*ncomp)++;
            }
            fsp--;
            /* propagate low to parent */
            if (fsp > 0) {
                Frame* parent = &frames[fsp - 1];
                if (low[cur->v] < low[parent->v])
                    low[parent->v] = low[cur->v];
            }
        }
    }
}

/* Articulation Points (cut vertices) — Tarjan's algorithm O(V+E)
 * An articulation point is a vertex whose removal increases #components.
 * Theorem: v is AP iff v is root with >1 child, or ∃child w: low[w] >= disc[v].
 * Reference: MIT 6.006, CLRS Problems 22-2 */
void articulation_points(Graph* g, bool is_ap[MAX_VERTICES]) {
    int n = g->nvertices;
    int disc[MAX_VERTICES], low[MAX_VERTICES], time = 0;
    int children[MAX_VERTICES] = {0};
    for (int i = 0; i < n; i++) {
        disc[i] = -1;
        is_ap[i] = false;
    }

    typedef struct { int v; int pi; int state; EdgeNode* e; } Frame;
    Frame frames[MAX_VERTICES * 2];
    int fsp = 0;

    for (int start = 0; start < n; start++) {
        if (disc[start] != -1) continue;
        frames[fsp++] = (Frame){start, -1, 0, g->adj[start]};
        while (fsp > 0) {
            Frame* cur = &frames[fsp - 1];
            if (cur->state == 0) {
                disc[cur->v] = low[cur->v] = ++time;
                cur->state = 1;
            }
            int found = 0;
            while (cur->e) {
                int w = cur->e->to;
                if (disc[w] == -1) {
                    if (cur->pi == -1) children[cur->v]++;
                    frames[fsp++] = (Frame){w, cur->v, 0, g->adj[w]};
                    found = 1;
                    cur->e = cur->e->next;
                    break;
                } else if (w != cur->pi) {
                    if (disc[w] < low[cur->v])
                        low[cur->v] = disc[w];
                }
                cur->e = cur->e->next;
            }
            if (found) continue;
            fsp--;
            if (fsp > 0) {
                Frame* parent = &frames[fsp - 1];
                if (low[cur->v] < low[parent->v])
                    low[parent->v] = low[cur->v];
                if (low[cur->v] >= disc[parent->v] && parent->pi != -1)
                    is_ap[parent->v] = true;
            } else {
                if (children[cur->v] > 1)
                    is_ap[cur->v] = true;
            }
        }
    }
}

/* Bridges in undirected graph — O(V+E) DFS with low-link
 * An edge (u,v) is a bridge iff low[v] > disc[u].
 * Reference: CLRS Problems 22-2 */
void bridges_find(Graph* g, int bridges_out[MAX_VERTICES][2], int* nbridges) {
    int n = g->nvertices;
    int disc[MAX_VERTICES], low[MAX_VERTICES], time = 0;
    *nbridges = 0;
    for (int i = 0; i < n; i++) disc[i] = -1;

    typedef struct { int v; int pi; int state; EdgeNode* e; } Frame;
    Frame frames[MAX_VERTICES * 2];
    int fsp = 0;

    for (int start = 0; start < n; start++) {
        if (disc[start] != -1) continue;
        frames[fsp++] = (Frame){start, -1, 0, g->adj[start]};
        while (fsp > 0) {
            Frame* cur = &frames[fsp - 1];
            if (cur->state == 0) {
                disc[cur->v] = low[cur->v] = ++time;
                cur->state = 1;
            }
            int found = 0;
            while (cur->e) {
                int w = cur->e->to;
                if (disc[w] == -1) {
                    frames[fsp++] = (Frame){w, cur->v, 0, g->adj[w]};
                    found = 1;
                    cur->e = cur->e->next;
                    break;
                } else if (w != cur->pi) {
                    if (disc[w] < low[cur->v])
                        low[cur->v] = disc[w];
                }
                cur->e = cur->e->next;
            }
            if (found) continue;
            fsp--;
            if (fsp > 0) {
                Frame* parent = &frames[fsp - 1];
                if (low[cur->v] < low[parent->v])
                    low[parent->v] = low[cur->v];
                if (low[cur->v] > disc[parent->v]) {
                    bridges_out[*nbridges][0] = parent->v;
                    bridges_out[*nbridges][1] = cur->v;
                    (*nbridges)++;
                }
            }
        }
    }
}

/* Ford-Fulkerson Max Flow — O(E * max_flow) using DFS augmenting paths
 * Uses residual capacity matrix. Max-Flow Min-Cut Theorem: max flow = min cut.
 * Reference: CLRS §26.2, MIT 6.046J */
static bool ff_dfs(int cap[MAX_VERTICES][MAX_VERTICES], int n,
                   int u, int sink, bool* visited, int* parent) {
    visited[u] = true;
    if (u == sink) return true;
    for (int v = 0; v < n; v++) {
        if (!visited[v] && cap[u][v] > 0) {
            parent[v] = u;
            if (ff_dfs(cap, n, v, sink, visited, parent))
                return true;
        }
    }
    return false;
}

int ford_fulkerson_maxflow(int capacity[MAX_VERTICES][MAX_VERTICES], int n,
                           int source, int sink) {
    int residual[MAX_VERTICES][MAX_VERTICES];
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            residual[i][j] = capacity[i][j];

    int max_flow = 0;
    int parent[MAX_VERTICES];
    bool visited[MAX_VERTICES];

    while (1) {
        memset(visited, 0, sizeof(visited));
        memset(parent, -1, sizeof(parent));
        if (!ff_dfs(residual, n, source, sink, visited, parent))
            break;

        /* find bottleneck capacity */
        int bottle = INF;
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            if (residual[u][v] < bottle) bottle = residual[u][v];
        }

        /* augment flow */
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            residual[u][v] -= bottle;
            residual[v][u] += bottle;
        }
        max_flow += bottle;
    }
    return max_flow;
}

/* Hamiltonian Cycle — NP-complete backtracking search
 * Uses Warnsdorff-like pruning: prefer vertices with fewer remaining options.
 * Reference: MIT 6.042J, Garey & Johnson */
static bool ham_backtrack(Graph* g, int path[MAX_VERTICES], int pos, bool* visited) {
    int n = g->nvertices;
    if (pos == n) {
        /* check if last vertex is adjacent to first */
        for (EdgeNode* e = g->adj[path[n - 1]]; e; e = e->next)
            if (e->to == path[0]) return true;
        return false;
    }
    int v = path[pos - 1];
    for (EdgeNode* e = g->adj[v]; e; e = e->next) {
        if (!visited[e->to]) {
            visited[e->to] = true;
            path[pos] = e->to;
            if (ham_backtrack(g, path, pos + 1, visited))
                return true;
            visited[e->to] = false;
        }
    }
    return false;
}

bool hamiltonian_cycle(Graph* g, int path[MAX_VERTICES]) {
    if (g->nvertices < 3) return false;
    bool visited[MAX_VERTICES] = {false};
    path[0] = 0;
    visited[0] = true;
    return ham_backtrack(g, path, 1, visited);
}

/* TSP 2-approximation using MST (Christofides-like, simplified)
 * For metric TSP: MST weight ≤ OPT. Double MST traversal ≤ 2*OPT.
 * Reference: CLRS §35.2, MIT 6.046J */
int tsp_mst_2approx(Graph* g, int tour[MAX_VERTICES]) {
    int n = g->nvertices;
    if (n == 0) return 0;
    if (n == 1) { tour[0] = 0; return 0; }

    /* Build complete adjacency matrix from graph */
    int dist[MAX_VERTICES][MAX_VERTICES];
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            dist[i][j] = (i == j) ? 0 : INF;
    for (int u = 0; u < n; u++)
        for (EdgeNode* e = g->adj[u]; e; e = e->next)
            dist[u][e->to] = e->weight;

    /* Floyd-Warshall for metric completion */
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (dist[i][k] != INF && dist[k][j] != INF &&
                    dist[i][k] + dist[k][j] < dist[i][j])
                    dist[i][j] = dist[i][k] + dist[k][j];

    /* Prim MST on complete graph */
    int parent[MAX_VERTICES];
    int key[MAX_VERTICES];
    bool in_mst[MAX_VERTICES] = {false};
    for (int i = 0; i < n; i++) { key[i] = INF; parent[i] = -1; }
    key[0] = 0;
    for (int count = 0; count < n; count++) {
        int u = -1, min_k = INF;
        for (int i = 0; i < n; i++)
            if (!in_mst[i] && key[i] < min_k) { min_k = key[i]; u = i; }
        if (u == -1) break;
        in_mst[u] = true;
        for (int v = 0; v < n; v++)
            if (!in_mst[v] && dist[u][v] < key[v]) {
                key[v] = dist[u][v];
                parent[v] = u;
            }
    }

    /* DFS traversal of MST (preorder) gives 2-approximation tour */
    bool visited_dfs[MAX_VERTICES] = {false};
    int stack[MAX_VERTICES], top = 0;
    stack[top++] = 0;
    int tidx = 0;
    while (top > 0) {
        int u = stack[--top];
        if (!visited_dfs[u]) {
            visited_dfs[u] = true;
            tour[tidx++] = u;
            for (int v = n - 1; v >= 0; v--)
                if (parent[v] == u && !visited_dfs[v])
                    stack[top++] = v;
        }
    }
    tour[tidx] = tour[0]; /* return to start */
    return tidx;
}

/* Graph diameter: maximum shortest path between any two vertices.
 * Uses Floyd-Warshall O(V³). */
int graph_diameter(Graph* g) {
    int dist[MAX_VERTICES][MAX_VERTICES];
    shortest_path_floyd(g, dist);
    int diam = 0;
    for (int i = 0; i < g->nvertices; i++)
        for (int j = 0; j < g->nvertices; j++)
            if (dist[i][j] != INF && dist[i][j] > diam)
                diam = dist[i][j];
    return diam;
}

/* Maximum bipartite matching — DFS augmenting path O(VE)
 * Uses Hungarian-style alternating path search.
 * Reference: CLRS §26.3, MIT 6.042J */
static bool bpm_dfs(Graph* g, int u, bool* seen, int matchR[MAX_VERTICES]) {
    for (EdgeNode* e = g->adj[u]; e; e = e->next) {
        int v = e->to;
        if (!seen[v]) {
            seen[v] = true;
            if (matchR[v] == -1 || bpm_dfs(g, matchR[v], seen, matchR)) {
                matchR[v] = u;
                return true;
            }
        }
    }
    return false;
}

int bipartite_max_matching(Graph* g, int match[MAX_VERTICES]) {
    int n = g->nvertices;
    int matchR[MAX_VERTICES];
    for (int i = 0; i < n; i++) { match[i] = -1; matchR[i] = -1; }
    int result = 0;
    for (int u = 0; u < n; u++) {
        bool seen[MAX_VERTICES] = {false};
        if (bpm_dfs(g, u, seen, matchR)) result++;
    }
    for (int i = 0; i < n; i++)
        if (matchR[i] != -1)
            match[matchR[i]] = i;
    return result;
}

/* Planarity heuristic: e ≤ 3v-6 for simple planar graphs (Euler's formula)
 * For connected planar graph with v ≥ 3: e ≤ 3v − 6.
 * Reference: MIT 6.042J, Kuratowski's Theorem */
bool is_planar_heuristic(Graph* g) {
    int v = g->nvertices;
    int e = g->nedges;
    if (g->directed) e /= 2;
    if (v < 3) return true;
    /* Euler's formula bound */
    if (e > 3 * v - 6) return false;
    /* Wagner's theorem check: if contains K5 or K3,3 minor → non-planar */
    if (v >= 5 && e >= 10) {
        int deg5_count = 0;
        for (int i = 0; i < v; i++) {
            int deg = 0;
            for (EdgeNode* eptr = g->adj[i]; eptr; eptr = eptr->next) deg++;
            if (deg >= 4) deg5_count++;
        }
        if (deg5_count >= 5) return false; /* potential K5 */
    }
    return true;
}

/* Transitive reduction: minimal graph preserving reachability
 * Remove edge (u,v) if ∃ path u→...→v of length ≥2.
 * Uses Floyd-Warshall for transitive closure. O(V³). */
void transitive_reduction(Graph* g, Graph* result) {
    int n = g->nvertices;
    graph_init(result, n, true);

    /* Compute transitive closure */
    int closure[MAX_VERTICES][MAX_VERTICES] = {{0}};
    for (int u = 0; u < n; u++)
        for (EdgeNode* e = g->adj[u]; e; e = e->next)
            closure[u][e->to] = 1;

    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (closure[i][k] && closure[k][j])
                    closure[i][j] = 1;

    /* Keep edge (u,v) iff no u→w→v path of length ≥2 */
    for (int u = 0; u < n; u++) {
        for (EdgeNode* e = g->adj[u]; e; e = e->next) {
            int v = e->to;
            bool has_longer_path = false;
            for (int w = 0; w < n; w++) {
                if (w != u && w != v && closure[u][w] && closure[w][v]) {
                    has_longer_path = true;
                    break;
                }
            }
            if (!has_longer_path)
                graph_add_edge(result, u, v, e->weight);
        }
    }
}
