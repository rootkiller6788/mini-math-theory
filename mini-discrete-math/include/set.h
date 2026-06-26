#ifndef SET_H
#define SET_H

#include <stdbool.h>

#define MAX_SET_SIZE 256

typedef struct {
    int elements[MAX_SET_SIZE];
    int size;
} Set;

void set_init(Set* s);
void set_add(Set* s, int x);
void set_remove(Set* s, int x);
bool set_contains(Set* s, int x);
int set_size(Set* s);
void set_print(Set* s);

Set set_union(Set* a, Set* b);
Set set_intersection(Set* a, Set* b);
Set set_difference(Set* a, Set* b);
bool set_subset(Set* a, Set* b);
bool set_equal(Set* a, Set* b);

typedef struct {
    Set sets[MAX_SET_SIZE];
    int count;
} PowerSet;
PowerSet power_set_generate(Set* s);

typedef struct {
    int first;
    int second;
} Pair;

typedef struct {
    Pair pairs[MAX_SET_SIZE * MAX_SET_SIZE];
    int size;
} CartesianProduct;
CartesianProduct cartesian_product(Set* a, Set* b);

/* Symmetric difference: A Δ B = (A \ B) ∪ (B \ A) */
Set symmetric_difference(Set* a, Set* b);

/* De Morgan's Laws verification in a given universe
 * Returns true if ¬(A∪B) = ¬A ∩ ¬B and ¬(A∩B) = ¬A ∪ ¬B hold */
bool verify_de_morgan(Set* a, Set* b, Set* universe);

/* Multiset (bag): elements with multiplicity */
typedef struct {
    int elements[MAX_SET_SIZE];
    int counts[MAX_SET_SIZE];
    int distinct;
} Multiset;

void multiset_init(Multiset* ms);
void multiset_add(Multiset* ms, int x);
void multiset_remove(Multiset* ms, int x);
int multiset_count(Multiset* ms, int x);
int multiset_total(Multiset* ms);
Multiset multiset_union(Multiset* a, Multiset* b);    /* max count */
Multiset multiset_intersection(Multiset* a, Multiset* b); /* min count */
Multiset multiset_sum(Multiset* a, Multiset* b);       /* sum counts */

/* Set partition: split a set into k non-empty disjoint subsets */
typedef struct {
    int subsets[MAX_SET_SIZE][MAX_SET_SIZE];
    int subset_sizes[MAX_SET_SIZE];
    int n_subsets;
} SetPartition;

/* Generate all set partitions (Bell number enumeration) */
void set_partitions_generate(Set* s, void (*callback)(SetPartition*));

/* Verify anti-chain property: no element is subset of another */
bool is_antichain(PowerSet* ps);

/* Set-theoretic Russell's Paradox demonstrator */
void russell_paradox_demo(void);

#endif
