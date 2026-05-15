#include "heap.h"
#include <stdlib.h>

static void swap(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}

static int parent(int i) { return (i - 1) / 2; }
static int left_child(int i) { return 2 * i + 1; }
static int right_child(int i) { return 2 * i + 2; }

Heap* heap_create(int capacity, bool is_min) {
    Heap* h = (Heap*)malloc(sizeof(Heap));
    h->data = (int*)malloc(sizeof(int) * capacity);
    h->size = 0;
    h->capacity = capacity;
    h->is_min_heap = is_min;
    return h;
}

void heap_free(Heap* h) {
    free(h->data);
    free(h);
}

static bool heap_compare(Heap* h, int a, int b) {
    if (h->is_min_heap) {
        return a < b;
    }
    return a > b;
}

static void bubble_up(Heap* h, int idx) {
    while (idx > 0 && heap_compare(h, h->data[idx], h->data[parent(idx)])) {
        swap(&h->data[idx], &h->data[parent(idx)]);
        idx = parent(idx);
    }
}

static void bubble_down(Heap* h, int idx) {
    while (1) {
        int target = idx;
        int l = left_child(idx);
        int r = right_child(idx);
        if (l < h->size && heap_compare(h, h->data[l], h->data[target])) {
            target = l;
        }
        if (r < h->size && heap_compare(h, h->data[r], h->data[target])) {
            target = r;
        }
        if (target == idx) break;
        swap(&h->data[idx], &h->data[target]);
        idx = target;
    }
}

void heap_push(Heap* h, int val) {
    h->data[h->size] = val;
    bubble_up(h, h->size);
    h->size++;
}

int heap_pop(Heap* h) {
    int root = h->data[0];
    h->data[0] = h->data[--(h->size)];
    bubble_down(h, 0);
    return root;
}

int heap_peek(Heap* h) {
    return h->data[0];
}

int heap_size(Heap* h) {
    return h->size;
}

bool heap_is_empty(Heap* h) {
    return h->size == 0;
}

static void sift_down(int* arr, int n, int i, bool is_min) {
    while (1) {
        int target = i;
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        if (is_min) {
            if (l < n && arr[l] < arr[target]) target = l;
            if (r < n && arr[r] < arr[target]) target = r;
        } else {
            if (l < n && arr[l] > arr[target]) target = l;
            if (r < n && arr[r] > arr[target]) target = r;
        }
        if (target == i) break;
        swap(&arr[i], &arr[target]);
        i = target;
    }
}

void heap_heapify(int* arr, int n, bool is_min) {
    for (int i = n / 2 - 1; i >= 0; i--) {
        sift_down(arr, n, i, is_min);
    }
}

void heapsort(int* arr, int n) {
    heap_heapify(arr, n, false);
    for (int i = n - 1; i > 0; i--) {
        swap(&arr[0], &arr[i]);
        sift_down(arr, i, 0, false);
    }
}
