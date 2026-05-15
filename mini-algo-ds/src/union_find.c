#include "union_find.h"
#include <stdlib.h>

UnionFind* uf_create(int n) {
    UnionFind* uf = (UnionFind*)malloc(sizeof(UnionFind));
    uf->parent = (int*)malloc(sizeof(int) * n);
    uf->rank = (int*)malloc(sizeof(int) * n);
    uf->n = n;
    for (int i = 0; i < n; i++) {
        uf->parent[i] = i;
        uf->rank[i] = 0;
    }
    return uf;
}

void uf_free(UnionFind* uf) {
    free(uf->parent);
    free(uf->rank);
    free(uf);
}

int uf_find(UnionFind* uf, int x) {
    if (uf->parent[x] != x) {
        uf->parent[x] = uf_find(uf, uf->parent[x]);
    }
    return uf->parent[x];
}

void uf_union(UnionFind* uf, int x, int y) {
    int rx = uf_find(uf, x);
    int ry = uf_find(uf, y);
    if (rx == ry) return;
    if (uf->rank[rx] < uf->rank[ry]) {
        uf->parent[rx] = ry;
    } else if (uf->rank[rx] > uf->rank[ry]) {
        uf->parent[ry] = rx;
    } else {
        uf->parent[ry] = rx;
        uf->rank[rx]++;
    }
}

bool uf_connected(UnionFind* uf, int x, int y) {
    return uf_find(uf, x) == uf_find(uf, y);
}

int uf_count_sets(UnionFind* uf) {
    int count = 0;
    for (int i = 0; i < uf->n; i++) {
        if (uf->parent[i] == i) count++;
    }
    return count;
}
