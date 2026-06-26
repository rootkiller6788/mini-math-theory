#include <stdio.h>
#include <stdlib.h>

#include "union_find.h"

static void print_array(const char *label, const int *arr, int n)
{
    printf("%s: [", label);
    for (int i = 0; i < n; i++)
        printf("%d%s", arr[i], i < n - 1 ? ", " : "");
    printf("]\n");
}

int main(void)
{
    printf("=== Union-Find (Disjoint Set Union) Demo ===\n\n");

    int n = 10;
    UnionFind *uf = uf_create(n);
    printf("Created %d elements (0..%d)\n\n", n, n - 1);

    /* Union operations */
    printf("--- Union operations ---\n");
    printf("uf_union(0, 1)\n"); uf_union(uf, 0, 1);
    printf("uf_union(1, 2)\n"); uf_union(uf, 1, 2);
    printf("uf_union(3, 4)\n"); uf_union(uf, 3, 4);
    printf("uf_union(5, 6)\n"); uf_union(uf, 5, 6);
    printf("uf_union(6, 7)\n"); uf_union(uf, 6, 7);
    printf("uf_union(7, 8)\n"); uf_union(uf, 7, 8);
    printf("uf_union(1, 3)\n\n"); uf_union(uf, 1, 3);

    /* Connectivity checks */
    printf("--- Connectivity checks ---\n");
    bool c1 = uf_connected(uf, 0, 4);
    printf("connected(0, 4) = %s\n", c1 ? "YES" : "NO");
    printf("Expected: YES (0-1-2-3-4 are all connected)\n");

    bool c2 = uf_connected(uf, 0, 9);
    printf("connected(0, 9) = %s\n", c2 ? "YES" : "NO");
    printf("Expected: NO (9 is isolated)\n\n");

    /* Count sets */
    printf("--- Count sets ---\n");
    int sets = uf_count_sets(uf);
    printf("Number of disjoint sets: %d\n", sets);
    printf("Expected: 3 sets:\n");
    printf("  Set 1: {0, 1, 2, 3, 4}\n");
    printf("  Set 2: {5, 6, 7, 8}\n");
    printf("  Set 3: {9}\n\n");

    /* Print per-element info */
    printf("--- Per-element information ---\n");
    printf("Element | Root\n");
    printf("--------+------\n");
    for (int i = 0; i < n; i++) {
        int parent = uf_find(uf, i);
        printf("   %d    |   %d\n", i, parent);
    }
    printf("\n");

    /* Show set membership */
    printf("--- Set membership ---\n");
    for (int i = 0; i < n; i++) {
        int root = uf_find(uf, i);
        printf("  Element %d belongs to set rooted at %d\n", i, root);
    }

    /* Print parent array using utility function */
    int parents[10];
    for (int i = 0; i < n; i++) parents[i] = uf_find(uf, i);
    print_array("Parent roots", parents, n);

    uf_free(uf);
    return 0;
}
