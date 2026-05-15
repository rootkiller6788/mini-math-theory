#ifndef SORT_H
#define SORT_H

#include <stdbool.h>

void quicksort(int* arr, int lo, int hi);
void mergesort(int* arr, int lo, int hi);
void insertionsort(int* arr, int n);
void selectionsort(int* arr, int n);
void bubblesort(int* arr, int n);
void countingsort(int* arr, int n, int max_val);
void radixsort(int* arr, int n);
int partition(int* arr, int lo, int hi);
int* kth_smallest(int* arr, int n, int k);
bool is_sorted(int* arr, int n);
void shuffle(int* arr, int n);

#endif
