/**
 * Vector: Dynamic Array Implementation
 *
 * Uses geometric resizing (doubling) for amortized O(1) push_back.
 * Aggregate analysis: total cost of n operations is O(n).
 * Each element is moved O(log n) times across all resizes.
 */

#include "vector.h"
#include "sort.h"
#include <stdlib.h>
#include <string.h>

Vector* vec_create(int initial_capacity) {
    Vector* v = (Vector*)malloc(sizeof(Vector));
    if (initial_capacity < 4) initial_capacity = 4;
    v->data = (int*)malloc(sizeof(int) * (size_t)initial_capacity);
    v->size = 0;
    v->capacity = initial_capacity;
    return v;
}

void vec_free(Vector* v) {
    if (v) {
        free(v->data);
        free(v);
    }
}

static void vec_grow(Vector* v, int min_capacity) {
    int new_cap = v->capacity * 2;
    if (new_cap < min_capacity) new_cap = min_capacity;
    int* new_data = (int*)realloc(v->data, sizeof(int) * (size_t)new_cap);
    if (new_data) {
        v->data = new_data;
        v->capacity = new_cap;
    }
}

void vec_push_back(Vector* v, int val) {
    if (v->size >= v->capacity) vec_grow(v, v->size + 1);
    v->data[v->size++] = val;
}

int vec_pop_back(Vector* v) {
    return v->data[--(v->size)];
}

int vec_get(Vector* v, int index) {
    return v->data[index];
}

void vec_set(Vector* v, int index, int val) {
    v->data[index] = val;
}

int vec_size(Vector* v) { return v->size; }
int vec_capacity(Vector* v) { return v->capacity; }
bool vec_is_empty(Vector* v) { return v->size == 0; }

void vec_reserve(Vector* v, int new_capacity) {
    if (new_capacity > v->capacity) vec_grow(v, new_capacity);
}

void vec_shrink_to_fit(Vector* v) {
    if (v->size < v->capacity) {
        v->data = (int*)realloc(v->data, sizeof(int) * (size_t)v->size);
        v->capacity = v->size;
    }
}

void vec_clear(Vector* v) {
    v->size = 0;
}

void vec_extend(Vector* v, const int* arr, int n) {
    if (v->size + n > v->capacity) vec_grow(v, v->size + n);
    memcpy(v->data + v->size, arr, sizeof(int) * (size_t)n);
    v->size += n;
}

Vector* vec_copy(const Vector* v) {
    Vector* c = vec_create(v->capacity);
    memcpy(c->data, v->data, sizeof(int) * (size_t)v->size);
    c->size = v->size;
    return c;
}

int vec_find(Vector* v, int val) {
    for (int i = 0; i < v->size; i++) {
        if (v->data[i] == val) return i;
    }
    return -1;
}

void vec_reverse(Vector* v) {
    for (int i = 0; i < v->size / 2; i++) {
        int tmp = v->data[i];
        v->data[i] = v->data[v->size - 1 - i];
        v->data[v->size - 1 - i] = tmp;
    }
}

void vec_sort(Vector* v) {
    if (v->size > 1) quicksort(v->data, 0, v->size - 1);
}

int vec_binary_search(Vector* v, int target) {
    int lo = 0, hi = v->size - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (v->data[mid] == target) return mid;
        if (v->data[mid] < target) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

void vec_foreach(Vector* v, vec_foreach_fn fn, void* ctx) {
    for (int i = 0; i < v->size; i++) {
        fn(v->data[i], i, ctx);
    }
}

void vec_push(Vector* v, int val)  { vec_push_back(v, val); }
int  vec_pop(Vector* v)            { return vec_pop_back(v); }
int  vec_top(Vector* v)            { return v->data[v->size - 1]; }
