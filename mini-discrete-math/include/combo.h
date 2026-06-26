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

/* Bell numbers B_n = number of set partitions of n elements */
long long bell_number(int n);

/* Inclusion-Exclusion principle for finite sets */
long long inclusion_exclusion(int set_sizes[], int n_sets, int universe);

/* Pigeonhole Principle: ensure at least n+1 items → two share a hole */
bool pigeonhole_check(int items[], int n_items, int n_holes);

/* Stirling numbers of the first kind s(n,k) — signless */
long long stirling_first(int n, int k);

/* Eulerian numbers A(n,k): permutations of {1..n} with k ascents */
long long eulerian_number(int n, int k);

/* Multinomial coefficient: n!/(k1! * k2! * ... * km!) */
long long multinomial(int n, int k[], int m);

/* Narayana numbers: N(n,k) = (1/n)*C(n,k)*C(n,k-1) */
long long narayana_number(int n, int k);

/* Burnside's Lemma: number of necklaces of length n with k colors */
long long necklaces_count(int n, int k);

/* Twelvefold Way: functions f: N→X (3 conditions × 4 equivalences)
 * Case: count of surjective functions from n-set to k-set */
long long surjective_count(int n, int k);

/* Integer composition: ordered partitions of n into exactly k parts */
long long compositions_count(int n, int k);

/* Partition function p(n) using Euler's pentagonal number theorem */
long long partition_pentagonal(int n);

/* Central binomial coefficient C(2n, n) */
long long central_binomial(int n);

/* Wedderburn-Etherington numbers: number of binary trees with n leaves */
long long wedderburn_etherington(int n);

#endif
