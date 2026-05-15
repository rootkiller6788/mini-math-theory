#include "invariants.h"
#include <stdlib.h>
#include <string.h>

void loop_inv_init(LoopInvariant* inv, const char* name, InvCheck check) {
    inv->name = name;
    inv->check = check;
    inv->nchecks = 0;
    inv->nviolations = 0;
    inv->established = false;
    inv->preserved = false;
}

bool loop_inv_establish(LoopInvariant* inv, void* state) {
    inv->nchecks++;
    if (!inv->check(state)) {
        inv->nviolations++;
        fprintf(stderr, "INVARIANT '%s': FAILED at establishment\n", inv->name);
        return false;
    }
    inv->established = true;
    return true;
}

bool loop_inv_maintain(LoopInvariant* inv, void* state) {
    inv->nchecks++;
    if (!inv->check(state)) {
        inv->nviolations++;
        fprintf(stderr, "INVARIANT '%s': FAILED during iteration\n", inv->name);
        return false;
    }
    inv->preserved = true;
    return true;
}

void loop_inv_report(LoopInvariant* inv) {
    printf("  Invariant '%s': %d checks, %d violations, estab=%s, preserv=%s\n",
           inv->name, inv->nchecks, inv->nviolations,
           inv->established ? "YES" : "NO",
           inv->preserved ? "YES" : "NO");
}

void invset_init(InvariantSet* is, int capacity) {
    is->invariants = malloc(capacity * sizeof(LoopInvariant));
    is->count = 0;
    is->capacity = capacity;
}

void invset_add(InvariantSet* is, const char* name, InvCheck check) {
    if (is->count >= is->capacity) return;
    loop_inv_init(&is->invariants[is->count++], name, check);
}

bool invset_check_all(InvariantSet* is, void* state) {
    bool ok = true;
    for (int i = 0; i < is->count; i++)
        if (!is->invariants[i].check(state)) {
            fprintf(stderr, "Invariant '%s' violated\n", is->invariants[i].name);
            ok = false;
        }
    return ok;
}

void invset_report(InvariantSet* is) {
    for (int i = 0; i < is->count; i++)
        loop_inv_report(&is->invariants[i]);
}
