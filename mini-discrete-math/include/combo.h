#ifndef COMBO_H
#define COMBO_H

long long factorial(int n);
long long permutation(int n, int r);
long long combination(int n, int r);

long long binomial_coefficient(int n, int k);
void pascal_triangle(int n, long long** triangle);

void permutations_generate(int* arr, int n, void (*callback)(int*, int));
void combinations_generate(int* arr, int n, int r, void (*callback)(int*, int));

void stirling_numbers_second(int n, int k, long long* result);
long long catalan_number(int n);

long long partitions_of_integer(int n);
void partitions_enumerate(int n, int max_part, int* buf, int buf_idx, void (*callback)(int*, int));

long long derangements(int n);

#endif
