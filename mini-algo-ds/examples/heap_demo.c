#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "heap.h"

static void print_array(const char *label, const int *arr, int n)
{
    printf("%s: [", label);
    for (int i = 0; i < n; i++)
        printf("%d%s", arr[i], i < n - 1 ? ", " : "");
    printf("]\n");
}

int main(void)
{
    srand((unsigned)time(NULL));

    printf("=== Heap Demo ===\n\n");

    /* --- Min Heap --- */
    printf("--- Min Heap ---\n");
    Heap *min_heap = heap_create(16, true);
    printf("Push: 5, 3, 8, 1, 9, 6\n");
    heap_push(min_heap, 5);
    heap_push(min_heap, 3);
    heap_push(min_heap, 8);
    heap_push(min_heap, 1);
    heap_push(min_heap, 9);
    heap_push(min_heap, 6);

    printf("Pop all (should be ascending): ");
    printf("[");
    int first = 1;
    while (!heap_is_empty(min_heap)) {
        if (!first) printf(", ");
        printf("%d", heap_pop(min_heap));
        first = 0;
    }
    printf("]\n");
    printf("Expected: [1, 3, 5, 6, 8, 9]\n\n");
    heap_free(min_heap);

    /* --- Max Heap --- */
    printf("--- Max Heap ---\n");
    Heap *max_heap = heap_create(16, false);
    printf("Push: 5, 3, 8, 1, 9, 6\n");
    heap_push(max_heap, 5);
    heap_push(max_heap, 3);
    heap_push(max_heap, 8);
    heap_push(max_heap, 1);
    heap_push(max_heap, 9);
    heap_push(max_heap, 6);

    printf("Pop all (should be descending): ");
    printf("[");
    first = 1;
    while (!heap_is_empty(max_heap)) {
        if (!first) printf(", ");
        printf("%d", heap_pop(max_heap));
        first = 0;
    }
    printf("]\n");
    printf("Expected: [9, 8, 6, 5, 3, 1]\n\n");
    heap_free(max_heap);

    /* --- Heapsort on random array --- */
    printf("--- Heapsort ---\n");
    int n = 10;
    int *data = (int *)malloc((size_t)n * sizeof(int));
    printf("Random array:  [");
    for (int i = 0; i < n; i++) {
        data[i] = rand() % 100;
        printf("%d%s", data[i], i < n - 1 ? ", " : "");
    }
    printf("]\n");

    heapsort(data, n);

    printf("After heapsort: [");
    for (int i = 0; i < n; i++)
        printf("%d%s", data[i], i < n - 1 ? ", " : "");
    printf("]\n\n");
    free(data);

    /* --- Heapify --- */
    printf("--- Heapify (unsorted -> min-heap) ---\n");
    int raw[] = {9, 3, 7, 1, 8, 2, 6, 4, 5};
    int raw_n = 9;
    print_array("Unsorted array", raw, raw_n);

    heap_heapify(raw, raw_n, true);

    Heap* h = heap_create(raw_n, true);
    h->size = raw_n;
    for (int i = 0; i < raw_n; i++) h->data[i] = raw[i];

    printf("After heapify, pop all: [");
    first = 1;
    while (!heap_is_empty(h)) {
        if (!first) printf(", ");
        printf("%d", heap_pop(h));
        first = 0;
    }
    printf("]\n");
    printf("Expected: [1, 2, 3, 4, 5, 6, 7, 8, 9]\n");
    heap_free(h);

    return 0;
}
