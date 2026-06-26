#ifndef RELATION_H
#define RELATION_H

#include <stdbool.h>
#include "set.h"

#define MAX_REL_SIZE 64

typedef struct {
    int matrix[MAX_REL_SIZE][MAX_REL_SIZE];
    int size; // 论域大小 {0, 1, ..., size-1}
} Relation;

void relation_init(Relation* r, int size);
void relation_add_pair(Relation* r, int a, int b);
bool relation_has(Relation* r, int a, int b);
void relation_print_matrix(Relation* r);
void relation_print_pairs(Relation* r);

bool relation_is_reflexive(Relation* r);
bool relation_is_symmetric(Relation* r);
bool relation_is_transitive(Relation* r);
bool relation_is_antisymmetric(Relation* r);
bool relation_is_equivalence(Relation* r);
bool relation_is_partial_order(Relation* r);

Relation relation_reflexive_closure(Relation* r);
Relation relation_symmetric_closure(Relation* r);
Relation relation_transitive_closure(Relation* r);    // Warshall

typedef struct {
    int classes[MAX_REL_SIZE][MAX_REL_SIZE];
    int class_sizes[MAX_REL_SIZE];
    int nclasses;
} EquivalencePartition;
EquivalencePartition equivalence_classes(Relation* r);

void hasse_diagram_print(Relation* r);

int* topological_sort(Relation* partial_order);

/* Lattice Theory — for partial orders that are lattices (join/meet) */

/* Lattice: PO where every pair has unique supremum (join) and infimum (meet) */
bool is_lattice(Relation* r);
int lattice_join(Relation* r, int a, int b);  /* returns -1 if no join */
int lattice_meet(Relation* r, int a, int b);  /* returns -1 if no meet */

/* Boolean lattice B^n: power set lattice with 2^n elements */
void boolean_lattice_generate(int n, Relation* r);

/* Check if lattice is distributive: x∧(y∨z) = (x∧y)∨(x∧z) */
bool is_distributive_lattice(Relation* r);

/* Check if lattice is complemented: every element has a complement */
bool is_complemented_lattice(Relation* r);

/* Dilworth's Theorem: In any finite PO, max antichain = min chain decomposition.
 * Returns size of maximum antichain via min path cover in bipartite graph.
 * Reference: Dilworth (1950) */
int dilworth_max_antichain(Relation* r);

/* Sperner's Theorem: Max antichain in B^n = C(n, floor(n/2)).
 * Returns binomial coefficient C(n, n/2). */
long long sperner_bound(int n);

/* Kleene closure (reflexive transitive closure then symmetric) — reachability */
Relation kleene_closure(Relation* r);

/* Warshall's algorithm for transitive closure (reference implementation) */
void warshall_transitive_closure(Relation* r, int result[MAX_REL_SIZE][MAX_REL_SIZE]);

/* Relation composition: R∘S = {(a,c) | ∃b: (a,b)∈R ∧ (b,c)∈S} */
Relation relation_compose(Relation* r, Relation* s);

/* Relation inverse: R^(-1) = {(b,a) | (a,b) ∈ R} */
Relation relation_inverse(Relation* r);

/* Check if relation is a total order (PO + total/comparability) */
bool relation_is_total_order(Relation* r);

/* Linear extension count (forecast for small n via backtracking) */
int count_linear_extensions(Relation* r);

#endif
