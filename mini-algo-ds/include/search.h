#ifndef SEARCH_H
#define SEARCH_H

int binary_search(int* arr, int n, int target);
int binary_search_first(int* arr, int n, int target);
int binary_search_last(int* arr, int n, int target);
int linear_search(int* arr, int n, int target);
int lower_bound(int* arr, int n, int target);
int upper_bound(int* arr, int n, int target);

#endif
