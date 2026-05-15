#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sort.h"
#include "heap.h"

#define NELEM(a) (sizeof(a) / sizeof((a)[0]))

static void print_array(const int *arr, int n)
{
    printf("[");
    for (int i = 0; i < n; i++)
        printf("%d%s", arr[i], i < n - 1 ? ", " : "");
    printf("]\n");
}

static int *copy_array(const int *src, int n)
{
    int *dst = (int *)malloc((size_t)n * sizeof(int));
    if (dst) memcpy(dst, src, (size_t)n * sizeof(int));
    return dst;
}

int main(void)
{
    int original[] = {64, 34, 25, 12, 22, 11, 90, 45, 33, 7};
    int n = NELEM(original);
    int *arr;
    clock_t start, end;
    double elapsed;

    printf("=== Sorting Algorithms Demo ===\n\n");
    printf("Original array: ");
    print_array(original, n);
    printf("\n");

    /* --- Quicksort --- */
    arr = copy_array(original, n);
    start = clock();
    quicksort(arr, 0, n - 1);
    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    printf("Quicksort:      ");
    print_array(arr, n);
    printf("  Time: %.3f ms\n\n", elapsed);
    free(arr);

    /* --- Mergesort --- */
    arr = copy_array(original, n);
    start = clock();
    mergesort(arr, 0, n - 1);
    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    printf("Mergesort:      ");
    print_array(arr, n);
    printf("  Time: %.3f ms\n\n", elapsed);
    free(arr);

    /* --- Heapsort --- */
    arr = copy_array(original, n);
    start = clock();
    heapsort(arr, n);
    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    printf("Heapsort:       ");
    print_array(arr, n);
    printf("  Time: %.3f ms\n\n", elapsed);
    free(arr);

    /* --- Insertion sort --- */
    arr = copy_array(original, n);
    start = clock();
    insertionsort(arr, n);
    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    printf("Insertionsort:  ");
    print_array(arr, n);
    printf("  Time: %.3f ms\n\n", elapsed);
    free(arr);

    /* --- Counting sort --- */
    arr = copy_array(original, n);
    start = clock();
    int maxv = arr[0]; for (int mi = 1; mi < n; mi++) if (arr[mi] > maxv) maxv = arr[mi];
    countingsort(arr, n, maxv);
    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    printf("Countingsort:   ");
    print_array(arr, n);
    printf("  Time: %.3f ms\n\n", elapsed);
    free(arr);

    /* --- Radix sort --- */
    arr = copy_array(original, n);
    start = clock();
    radixsort(arr, n);
    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    printf("Radixsort:      ");
    print_array(arr, n);
    printf("  Time: %.3f ms\n\n", elapsed);
    free(arr);

    /* --- Quickselect (kth_smallest) --- */
    printf("=== Quickselect (kth_smallest) ===\n");
    printf("Array: ");
    print_array(original, n);

    arr = copy_array(original, n);
    int* pk1 = kth_smallest(arr, n, 1);
    printf("k=1   (smallest):     %d\n", *pk1);

    arr = copy_array(original, n);
    int* pk2 = kth_smallest(arr, n, n / 2);
    printf("k=n/2 (median-ish):   %d\n", *pk2);

    arr = copy_array(original, n);
    int* pk3 = kth_smallest(arr, n, n);
    printf("k=n   (largest):      %d\n", *pk3);

    return 0;
}
