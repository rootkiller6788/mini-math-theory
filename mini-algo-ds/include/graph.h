/**
 * Graph ADT (Adjacency List) - L1: Core Definition, L3: Engineering Structure
 *
 * Implements adjacency list representation with O(V+E) space.
 * Supports both directed and undirected graphs with weighted edges.
 * 
 * CMU 15-210: Graph representations
 * MIT 6.006 Lecture 11: Graph Representations
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>

typedef struct Edge {
    int to;
    int weight;
    struct Edge* next;
} Edge;

typedef struct {
    Edge* head;
} AdjList;

typedef struct {
    int V;
    int E;
    bool directed;
    AdjList* adjacency;
} Graph;

Graph* g_create(int vertices, bool directed);
void g_free(Graph* g);
void g_add_edge(Graph* g, int from, int to, int weight);
void g_remove_edge(Graph* g, int from, int to);
bool g_has_edge(Graph* g, int from, int to);
int  g_vertex_count(Graph* g);
int  g_edge_count(Graph* g);
int  g_out_degree(Graph* g, int v);
int  g_in_degree(Graph* g, int v);
int  g_edge_weight(Graph* g, int from, int to);
int  g_neighbors(Graph* g, int v, int* buf, int bufsize);
void g_transpose(Graph* g, Graph* t);
int  g_connected_components(Graph* g, int* component);
bool g_is_bipartite(Graph* g, int* color);
Graph* g_reverse(Graph* g);
bool g_is_dag(Graph* g);

#endif
