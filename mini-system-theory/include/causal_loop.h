#ifndef CAUSAL_LOOP_H
#define CAUSAL_LOOP_H

typedef struct {
    int from, to;
    double polarity;
} CausalLink;

typedef struct {
    CausalLink* links;
    int n_links;
    int n_nodes;
    char** node_names;
} CausalLoopDiagram;

CausalLoopDiagram* cld_create(int n_nodes);
void cld_add_link(CausalLoopDiagram* cld, int from, int to, double polarity);
int cld_count_feedback_loops(CausalLoopDiagram* cld);
int cld_is_reinforcing(CausalLoopDiagram* cld, int* loop_nodes, int loop_len);
void cld_print_adjacency(CausalLoopDiagram* cld);
void cld_print_loops(CausalLoopDiagram* cld);
void cld_free(CausalLoopDiagram* cld);

#endif
