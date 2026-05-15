#include "greedy.h"
#include "heap.h"
#include <stdlib.h>
#include <string.h>

static void swap_int(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}

static void insertionsort_local(int* a, int n) {
    for (int i = 1; i < n; i++) {
        int key = a[i];
        int j = i - 1;
        while (j >= 0 && a[j] > key) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = key;
    }
}

static void sort_by_finish(int* start, int* finish, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (finish[j] > finish[j + 1]) {
                swap_int(&finish[j], &finish[j + 1]);
                swap_int(&start[j], &start[j + 1]);
            }
        }
    }
}

int activity_selection(int* start, int* finish, int n, int* selected) {
    int* s = (int*)malloc(sizeof(int) * n);
    int* f = (int*)malloc(sizeof(int) * n);
    memcpy(s, start, sizeof(int) * n);
    memcpy(f, finish, sizeof(int) * n);
    sort_by_finish(s, f, n);
    int count = 0;
    selected[count++] = 0;
    int last_finish = f[0];
    for (int i = 1; i < n; i++) {
        if (s[i] >= last_finish) {
            selected[count++] = i;
            last_finish = f[i];
        }
    }
    free(s);
    free(f);
    return count;
}

double fractional_knapsack(int* weights, int* values, int n, int capacity, double* fractions) {
    typedef struct { int weight; int value; double ratio; int idx; } Item;
    Item* items = (Item*)malloc(sizeof(Item) * n);
    for (int i = 0; i < n; i++) {
        items[i].weight = weights[i];
        items[i].value = values[i];
        items[i].ratio = (double)values[i] / weights[i];
        items[i].idx = i;
    }
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (items[j].ratio < items[j + 1].ratio) {
                Item t = items[j];
                items[j] = items[j + 1];
                items[j + 1] = t;
            }
        }
    }
    double total_value = 0.0;
    int remaining = capacity;
    for (int i = 0; i < n; i++) fractions[i] = 0.0;
    for (int i = 0; i < n && remaining > 0; i++) {
        if (items[i].weight <= remaining) {
            fractions[items[i].idx] = 1.0;
            total_value += items[i].value;
            remaining -= items[i].weight;
        } else {
            double frac = (double)remaining / items[i].weight;
            fractions[items[i].idx] = frac;
            total_value += items[i].value * frac;
            remaining = 0;
        }
    }
    free(items);
    return total_value;
}

int min_platforms(int* arrival, int* departure, int n) {
    int* arr = (int*)malloc(sizeof(int) * n);
    int* dep = (int*)malloc(sizeof(int) * n);
    memcpy(arr, arrival, sizeof(int) * n);
    memcpy(dep, departure, sizeof(int) * n);
    insertionsort_local(arr, n);
    insertionsort_local(dep, n);
    int platforms = 0, max_platforms = 0;
    int i = 0, j = 0;
    while (i < n && j < n) {
        if (arr[i] <= dep[j]) {
            platforms++;
            if (platforms > max_platforms) max_platforms = platforms;
            i++;
        } else {
            platforms--;
            j++;
        }
    }
    free(arr);
    free(dep);
    return max_platforms;
}

int huffman_coding_cost(int* freqs, int n) {
    Heap* h = heap_create(n + 1, true);
    for (int i = 0; i < n; i++) {
        heap_push(h, freqs[i]);
    }
    int total_cost = 0;
    while (heap_size(h) > 1) {
        int a = heap_pop(h);
        int b = heap_pop(h);
        int sum = a + b;
        total_cost += sum;
        heap_push(h, sum);
    }
    heap_free(h);
    return total_cost;
}
