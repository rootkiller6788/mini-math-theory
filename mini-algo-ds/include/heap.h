#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>

typedef struct {
    int* data;
    int size;
    int capacity;
    bool is_min_heap;
} Heap;

Heap* heap_create(int capacity, bool is_min);
void heap_free(Heap* h);
void heap_push(Heap* h, int val);
int heap_pop(Heap* h);
int heap_peek(Heap* h);
int heap_size(Heap* h);
bool heap_is_empty(Heap* h);
void heap_heapify(int* arr, int n, bool is_min);
void heapsort(int* arr, int n);

#endif
