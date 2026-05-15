#include "causal_loop.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PI 3.14159265358979323846

CausalLoopDiagram* cld_create(int n_nodes) {
    CausalLoopDiagram* cld = (CausalLoopDiagram*)malloc(sizeof(CausalLoopDiagram));
    cld->n_nodes = n_nodes;
    cld->n_links = 0;
    cld->links = NULL;
    cld->node_names = (char**)malloc(n_nodes * sizeof(char*));
    for (int i = 0; i < n_nodes; i++) {
        cld->node_names[i] = (char*)malloc(32 * sizeof(char));
        sprintf(cld->node_names[i], "Node_%d", i);
    }
    return cld;
}

void cld_add_link(CausalLoopDiagram* cld, int from, int to, double polarity) {
    cld->n_links++;
    cld->links = (CausalLink*)realloc(cld->links, cld->n_links * sizeof(CausalLink));
    cld->links[cld->n_links - 1].from = from;
    cld->links[cld->n_links - 1].to = to;
    cld->links[cld->n_links - 1].polarity = polarity;
}

static int dfs_count_loops(int** adj, int n, int start, int current, int* visited,
                           int depth, int max_depth) {
    if (depth > max_depth) return 0;
    int count = 0;
    for (int next = 0; next < n; next++) {
        if (adj[current][next] != 0) {
            if (next == start && depth >= 1) {
                count++;
                continue;
            }
            if (!visited[next]) {
                visited[next] = 1;
                count += dfs_count_loops(adj, n, start, next, visited, depth + 1, max_depth);
                visited[next] = 0;
            }
        }
    }
    return count;
}

int cld_count_feedback_loops(CausalLoopDiagram* cld) {
    int n = cld->n_nodes;
    int** adj = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        adj[i] = (int*)calloc(n, sizeof(int));
    }
    for (int k = 0; k < cld->n_links; k++) {
        int f = cld->links[k].from;
        int t = cld->links[k].to;
        adj[f][t] = 1;
    }
    int total = 0;
    int* visited = (int*)calloc(n, sizeof(int));
    for (int i = 0; i < n; i++) {
        visited[i] = 1;
        total += dfs_count_loops(adj, n, i, i, visited, 1, n);
        visited[i] = 0;
    }
    for (int i = 0; i < n; i++) free(adj[i]);
    free(adj);
    free(visited);
    return total;
}

int cld_is_reinforcing(CausalLoopDiagram* cld, int* loop_nodes, int loop_len) {
    double product = 1.0;
    for (int k = 0; k < loop_len; k++) {
        int from = loop_nodes[k];
        int to = loop_nodes[(k + 1) % loop_len];
        int found = 0;
        for (int l = 0; l < cld->n_links; l++) {
            if (cld->links[l].from == from && cld->links[l].to == to) {
                product *= cld->links[l].polarity;
                found = 1;
                break;
            }
        }
        if (!found) return 0;
    }
    return (product > 0) ? 1 : -1;
}

void cld_print_adjacency(CausalLoopDiagram* cld) {
    int n = cld->n_nodes;
    int** adj = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        adj[i] = (int*)calloc(n, sizeof(int));
    }
    for (int k = 0; k < cld->n_links; k++) {
        int f = cld->links[k].from;
        int t = cld->links[k].to;
        adj[f][t] = (cld->links[k].polarity > 0) ? 1 : -1;
    }
    printf("Adjacency Matrix (%dx%d):\n", n, n);
    printf("    ");
    for (int j = 0; j < n; j++) printf("%8s", cld->node_names[j]);
    printf("\n");
    for (int i = 0; i < n; i++) {
        printf("%8s", cld->node_names[i]);
        for (int j = 0; j < n; j++) {
            printf(" %+6d ", adj[i][j]);
        }
        printf("\n");
    }
    for (int i = 0; i < n; i++) free(adj[i]);
    free(adj);
}

void cld_print_loops(CausalLoopDiagram* cld) {
    int count = cld_count_feedback_loops(cld);
    printf("Causal Loop Diagram: %d nodes, %d links\n", cld->n_nodes, cld->n_links);
    printf("Total feedback loops found: %d\n", count);
    printf("Links:\n");
    for (int i = 0; i < cld->n_links; i++) {
        CausalLink* l = &cld->links[i];
        printf("  %s -> %s [%+d]\n",
               cld->node_names[l->from],
               cld->node_names[l->to],
               (int)l->polarity);
    }
}

void cld_free(CausalLoopDiagram* cld) {
    free(cld->links);
    for (int i = 0; i < cld->n_nodes; i++) free(cld->node_names[i]);
    free(cld->node_names);
    free(cld);
}
