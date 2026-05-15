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

#endif
