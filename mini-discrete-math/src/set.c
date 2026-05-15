#include "set.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int cmp(const void* a, const void* b) {
    return *(int*)a - *(int*)b;
}

void set_init(Set* s) {
    s->size = 0;
}

void set_add(Set* s, int x) {
    for (int i = 0; i < s->size; i++)
        if (s->elements[i] == x) return;
    s->elements[s->size++] = x;
    qsort(s->elements, s->size, sizeof(int), cmp);
}

void set_remove(Set* s, int x) {
    for (int i = 0; i < s->size; i++) {
        if (s->elements[i] == x) {
            for (int j = i; j < s->size - 1; j++)
                s->elements[j] = s->elements[j + 1];
            s->size--;
            return;
        }
    }
}

bool set_contains(Set* s, int x) {
    for (int i = 0; i < s->size; i++)
        if (s->elements[i] == x) return true;
    return false;
}

int set_size(Set* s) {
    return s->size;
}

void set_print(Set* s) {
    printf("{");
    for (int i = 0; i < s->size; i++) {
        printf("%d", s->elements[i]);
        if (i < s->size - 1) printf(", ");
    }
    printf("}");
}

Set set_union(Set* a, Set* b) {
    Set result;
    set_init(&result);
    int i = 0, j = 0;
    while (i < a->size && j < b->size) {
        if (a->elements[i] < b->elements[j])
            result.elements[result.size++] = a->elements[i++];
        else if (a->elements[i] > b->elements[j])
            result.elements[result.size++] = b->elements[j++];
        else {
            result.elements[result.size++] = a->elements[i++];
            j++;
        }
    }
    while (i < a->size) result.elements[result.size++] = a->elements[i++];
    while (j < b->size) result.elements[result.size++] = b->elements[j++];
    return result;
}

Set set_intersection(Set* a, Set* b) {
    Set result;
    set_init(&result);
    int i = 0, j = 0;
    while (i < a->size && j < b->size) {
        if (a->elements[i] < b->elements[j]) i++;
        else if (a->elements[i] > b->elements[j]) j++;
        else {
            result.elements[result.size++] = a->elements[i];
            i++; j++;
        }
    }
    return result;
}

Set set_difference(Set* a, Set* b) {
    Set result;
    set_init(&result);
    for (int i = 0; i < a->size; i++)
        if (!set_contains(b, a->elements[i]))
            result.elements[result.size++] = a->elements[i];
    return result;
}

bool set_subset(Set* a, Set* b) {
    for (int i = 0; i < a->size; i++)
        if (!set_contains(b, a->elements[i])) return false;
    return true;
}

bool set_equal(Set* a, Set* b) {
    return a->size == b->size && set_subset(a, b);
}

PowerSet power_set_generate(Set* s) {
    PowerSet ps;
    ps.count = 0;
    int total = 1 << s->size;
    for (int mask = 0; mask < total; mask++) {
        set_init(&ps.sets[mask]);
        for (int i = 0; i < s->size; i++)
            if (mask & (1 << i))
                ps.sets[mask].elements[ps.sets[mask].size++] = s->elements[i];
    }
    ps.count = total;
    return ps;
}

CartesianProduct cartesian_product(Set* a, Set* b) {
    CartesianProduct cp;
    cp.size = 0;
    for (int i = 0; i < a->size; i++)
        for (int j = 0; j < b->size; j++) {
            cp.pairs[cp.size].first = a->elements[i];
            cp.pairs[cp.size].second = b->elements[j];
            cp.size++;
        }
    return cp;
}
