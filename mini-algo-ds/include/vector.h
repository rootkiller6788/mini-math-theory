/**
 * mini-vector: Dynamic Array (L1: Core Definition, L3: Engineering Structure)
 *
 * Amortized O(1) push_back via geometric resizing (doubling strategy).
 * Implements the Array Doubling theorem: total cost of n insertions is O(n).
 *
 * MIT 6.006 Lecture 2: Data Structures and Dynamic Arrays
 * CLRS Chapter 17: Amortized Analysis - Aggregate Method
 */

#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>

typedef struct {
    int* data;
    int size;
    int capacity;
} Vector;

Vector* vec_create(int initial_capacity);
void vec_free(Vector* v);
void vec_push_back(Vector* v, int val);
int  vec_pop_back(Vector* v);
int  vec_get(Vector* v, int index);
void vec_set(Vector* v, int index, int val);
int  vec_size(Vector* v);
int  vec_capacity(Vector* v);
bool vec_is_empty(Vector* v);
void vec_reserve(Vector* v, int new_capacity);
void vec_shrink_to_fit(Vector* v);
void vec_clear(Vector* v);
void vec_extend(Vector* v, const int* arr, int n);
Vector* vec_copy(const Vector* v);
int  vec_find(Vector* v, int val);
void vec_reverse(Vector* v);
void vec_sort(Vector* v);
int  vec_binary_search(Vector* v, int target);
typedef void (*vec_foreach_fn)(int val, int index, void* ctx);
void vec_foreach(Vector* v, vec_foreach_fn fn, void* ctx);

#endif
