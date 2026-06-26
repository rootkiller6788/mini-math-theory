/**
 * Graph ADT Implementation (Adjacency List)
 * L1: Core Definition - vertex-edge-graph model
 * L3: Engineering Structure - adjacency list with O(V+E) space
 *
 * Supports directed/undirected, weighted edges.
 * CMU 15-210 / MIT 6.006
 */

#include "graph.h"
#include "queue.h"
#include <stdlib.h>
#include <string.h>

Graph* g_create(int vertices, bool directed) {
    Graph* g = (Graph*)malloc(sizeof(Graph));
    g->V = vertices;
    g->E = 0;
    g->directed = directed;
    g->adjacency = (AdjList*)calloc((size_t)vertices, sizeof(AdjList));
    return g;
}

void g_free(Graph* g) {
    if (!g) return;
    for (int i = 0; i < g->V; i++) {
        Edge* e = g->adjacency[i].head;
        while (e) { Edge* next = e->next; free(e); e = next; }
    }
    free(g->adjacency);
    free(g);
}

void g_add_edge(Graph* g, int from, int to, int weight) {
    Edge* e = (Edge*)malloc(sizeof(Edge));
    e->to = to;
    e->weight = weight;
    e->next = g->adjacency[from].head;
    g->adjacency[from].head = e;
    g->E++;
    if (!g->directed) {
        Edge* re = (Edge*)malloc(sizeof(Edge));
        re->to = from;
        re->weight = weight;
        re->next = g->adjacency[to].head;
        g->adjacency[to].head = re;
    }
}

void g_remove_edge(Graph* g, int from, int to) {
    Edge *cur = g->adjacency[from].head, *prev = NULL;
    while (cur) {
        if (cur->to == to) {
            if (prev) prev->next = cur->next;
            else g->adjacency[from].head = cur->next;
            free(cur);
            g->E--;
            break;
        }
        prev = cur;
        cur = cur->next;
    }
    if (!g->directed) {
        cur = g->adjacency[to].head; prev = NULL;
        while (cur) {
            if (cur->to == from) {
                if (prev) prev->next = cur->next;
                else g->adjacency[to].head = cur->next;
                free(cur);
                break;
            }
            prev = cur;
            cur = cur->next;
        }
    }
}

bool g_has_edge(Graph* g, int from, int to) {
    for (Edge* e = g->adjacency[from].head; e; e = e->next)
        if (e->to == to) return true;
    return false;
}

int g_edge_weight(Graph* g, int from, int to) {
    for (Edge* e = g->adjacency[from].head; e; e = e->next)
        if (e->to == to) return e->weight;
    return -1;
}

int g_vertex_count(Graph* g) { return g->V; }
int g_edge_count(Graph* g) { return g->directed ? g->E : g->E / 2; }

int g_out_degree(Graph* g, int v) {
    int d = 0;
    for (Edge* e = g->adjacency[v].head; e; e = e->next) d++;
    return d;
}

int g_in_degree(Graph* g, int v) {
    int d = 0;
    for (int i = 0; i < g->V; i++)
        for (Edge* e = g->adjacency[i].head; e; e = e->next)
            if (e->to == v) d++;
    return d;
}

int g_neighbors(Graph* g, int v, int* buf, int bufsize) {
    int count = 0;
    for (Edge* e = g->adjacency[v].head; e && count < bufsize; e = e->next)
        buf[count++] = e->to;
    return count;
}

/* Graph transpose: reverse all edge directions */
void g_transpose(Graph* g, Graph* t) {
    for (int u = 0; u < g->V; u++)
        for (Edge* e = g->adjacency[u].head; e; e = e->next)
            g_add_edge(t, e->to, u, e->weight);
}

Graph* g_reverse(Graph* g) {
    Graph* r = g_create(g->V, g->directed);
    for (int u = 0; u < g->V; u++)
        for (Edge* e = g->adjacency[u].head; e; e = e->next)
            g_add_edge(r, e->to, u, e->weight);
    return r;
}

/* Connected components via BFS (undirected only) */
int g_connected_components(Graph* g, int* component) {
    for (int i = 0; i < g->V; i++) component[i] = -1;
    int comp_id = 0;
    for (int v = 0; v < g->V; v++) {
        if (component[v] != -1) continue;
        int q[1024], head = 0, tail = 0;
        component[v] = comp_id;
        q[tail++] = v;
        while (head < tail) {
            int u = q[head++];
            for (Edge* e = g->adjacency[u].head; e; e = e->next) {
                if (component[e->to] == -1) {
                    component[e->to] = comp_id;
                    q[tail++] = e->to;
                }
            }
        }
        comp_id++;
    }
    return comp_id;
}

/* Bipartite check via BFS 2-coloring */
bool g_is_bipartite(Graph* g, int* color) {
    for (int i = 0; i < g->V; i++) color[i] = -1;
    for (int start = 0; start < g->V; start++) {
        if (color[start] != -1) continue;
        int q[1024], head = 0, tail = 0;
        color[start] = 0;
        q[tail++] = start;
        while (head < tail) {
            int v = q[head++];
            for (Edge* e = g->adjacency[v].head; e; e = e->next) {
                if (color[e->to] == -1) {
                    color[e->to] = 1 - color[v];
                    q[tail++] = e->to;
                } else if (color[e->to] == color[v]) {
                    return false;
                }
            }
        }
    }
    return true;
}

/* DAG check via cycle detection (DFS 3-color) */
static bool has_cycle_dfs(Graph* g, int v, int* state) {
    state[v] = 1;
    for (Edge* e = g->adjacency[v].head; e; e = e->next) {
        if (state[e->to] == 1) return true;
        if (state[e->to] == 0 && has_cycle_dfs(g, e->to, state)) return true;
    }
    state[v] = 2;
    return false;
}

bool g_is_dag(Graph* g) {
    int* state = (int*)calloc((size_t)g->V, sizeof(int));
    for (int i = 0; i < g->V; i++)
        if (state[i] == 0 && has_cycle_dfs(g, i, state)) {
            free(state);
            return false;
        }
    free(state);
    return true;
}
