#define CONTRACT_CHECK
#include "contracts.h"
#include "assertions.h"
#include "spec.h"
#include <stdio.h>
#include <string.h>

static void bubble_sort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (arr[j] > arr[j + 1]) {
                int t = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = t;
            }
}

bool is_sorted(int arr[], int n) {
    for (int i = 0; i < n - 1; i++)
        if (arr[i] > arr[i + 1]) return false;
    return true;
}

bool is_permutation(int a[], int b[], int n) {
    int count_a[1024] = {0}, count_b[1024] = {0};
    for (int i = 0; i < n; i++) {
        if (a[i] < 0 || a[i] >= 1024) return false;
        count_a[a[i]]++;
    }
    for (int i = 0; i < n; i++) {
        if (b[i] < 0 || b[i] >= 1024) return false;
        count_b[b[i]]++;
    }
    for (int i = 0; i < 1024; i++)
        if (count_a[i] != count_b[i]) return false;
    return true;
}

bool sort_pre_valid_input(void* input) {
    int* arr = ((int**)input)[0];
    int n = *((int*)((int**)input)[1]);
    return n > 0 && n < 1024;
}

bool sort_post_sorted(void* input_raw, void* output_raw) {
    int* out = (int*)output_raw;
    int n = *((int*)((int**)input_raw)[1]);
    return is_sorted(out, n);
}

bool sort_post_permutation(void* input_raw, void* output_raw) {
    int* in = ((int**)input_raw)[0];
    int n = *((int*)((int**)input_raw)[1]);
    int* out = (int*)output_raw;
    return is_permutation(in, out, n);
}

static void sort_wrapper(void* input, void* output) {
    int** args = (int**)input;
    int* arr = args[0];
    int n = *(int*)args[1];
    memcpy(output, arr, n * sizeof(int));
    bubble_sort((int*)output, n);
}

int main(void) {
    printf("====== Verified Sort Demo ======\n\n");

    printf("--- 1. Postcondition: sorted ---\n");
    int input1[] = {5, 2, 8, 1, 9, 3};
    int n1 = 6;
    int output1[6];

    bubble_sort(input1, n1);
    bool sorted = is_sorted(input1, n1);
    printf("Sorted? %s\n", sorted ? "YES" : "NO");
    for (int i = 0; i < n1; i++) printf("%d ", input1[i]);
    printf("\n\n");

    printf("--- 2. Postcondition: permutation ---\n");
    int input2[] = {7, 3, 5, 7, 3, 1};
    int n2 = 6;
    int sorted2[6];
    memcpy(sorted2, input2, n2 * sizeof(int));
    bubble_sort(sorted2, n2);
    printf("Is permutation of original? %s\n",
           is_permutation(input2, sorted2, n2) ? "YES" : "NO");
    printf("Original: ");
    for (int i = 0; i < n2; i++) printf("%d ", input2[i]);
    printf("\nSorted:   ");
    for (int i = 0; i < n2; i++) printf("%d ", sorted2[i]);
    printf("\n\n");

    printf("--- 3. Formal Spec Check ---\n");
    Spec sort_spec;
    spec_init(&sort_spec, "bubble_sort");
    spec_add_precondition(&sort_spec, sort_pre_valid_input, "n in (0, 1024)");
    spec_add_postcondition(&sort_spec, sort_post_sorted, "output is sorted");
    spec_add_postcondition(&sort_spec, sort_post_permutation, "output is permutation");

    int test_in[] = {9, 4, 7, 2, 8, 1, 6, 3};
    int n = 8;
    int test_out[8];
    void* spec_input[2] = { test_in, &n };
    spec_check(&sort_spec, spec_input, sort_wrapper, test_out);

    printf("--- 4. Assertion-based Test Suite ---\n");
    assert_eq_int(is_sorted(test_out, n), 1, "sorted after sort");
    assert_eq_int(is_permutation(test_in, test_out, n), 1, "permutation holds");

    int single[] = {42};
    bubble_sort(single, 1);
    assert_eq_int(single[0], 42, "single element unchanged");
    assert_eq_int(is_sorted(single, 1), 1, "single element is sorted");

    int already[] = {1, 2, 3, 4, 5};
    bubble_sort(already, 5);
    assert_eq_int(is_sorted(already, 5), 1, "already sorted stays sorted");
    assert_eq_int(is_permutation((int[]){1,2,3,4,5}, already, 5), 1, "still permutation");

    assert_summary();

    return 0;
}
