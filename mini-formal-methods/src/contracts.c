#include "contracts.h"

bool contract_check(Contract* c, void* input, void* output) {
    if (c->pre && !c->pre(input)) {
        fprintf(stderr, "CONTRACT: precondition failed\n");
        return false;
    }
    if (c->inv && !c->inv(input)) {
        fprintf(stderr, "CONTRACT: invariant failed before call\n");
        return false;
    }
    c->func(input);
    if (c->post && !c->post(input, output)) {
        fprintf(stderr, "CONTRACT: postcondition failed\n");
        return false;
    }
    if (c->inv && !c->inv(output)) {
        fprintf(stderr, "CONTRACT: invariant failed after call\n");
        return false;
    }
    return true;
}
