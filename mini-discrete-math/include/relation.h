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

#endif
