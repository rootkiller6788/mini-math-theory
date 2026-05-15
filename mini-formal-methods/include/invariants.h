#ifndef INVARIANTS_H
#define INVARIANTS_H

#include <stdbool.h>
#include <stdio.h>

typedef bool (*InvCheck)(void* state);

typedef struct {
    const char* name;
    InvCheck check;
    int nchecks;
    int nviolations;
    bool established;
    bool preserved;
} LoopInvariant;

void loop_inv_init(LoopInvariant* inv, const char* name, InvCheck check);
bool loop_inv_establish(LoopInvariant* inv, void* state);
bool loop_inv_maintain(LoopInvariant* inv, void* state);
void loop_inv_report(LoopInvariant* inv);

typedef struct {
    LoopInvariant* invariants;
    int count;
    int capacity;
} InvariantSet;

void invset_init(InvariantSet* is, int capacity);
void invset_add(InvariantSet* is, const char* name, InvCheck check);
bool invset_check_all(InvariantSet* is, void* state);
void invset_report(InvariantSet* is);

#endif
