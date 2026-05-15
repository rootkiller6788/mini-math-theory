#ifndef UNION_FIND_H
#define UNION_FIND_H

#include <stdbool.h>

typedef struct {
    int* parent;
    int* rank;
    int n;
} UnionFind;

UnionFind* uf_create(int n);
void uf_free(UnionFind* uf);
int uf_find(UnionFind* uf, int x);
void uf_union(UnionFind* uf, int x, int y);
bool uf_connected(UnionFind* uf, int x, int y);
int uf_count_sets(UnionFind* uf);

#endif
