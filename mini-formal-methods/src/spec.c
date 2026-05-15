#include "spec.h"
#include <stdio.h>
#include <string.h>

void spec_init(Spec* s, const char* name) {
    s->name = name;
    s->npre = 0;
    s->npost = 0;
    s->ninv = 0;
}

void spec_add_precondition(Spec* s, Precondition p, const char* name) {
    if (s->npre >= MAX_PRECONDITIONS) return;
    s->pre[s->npre] = p;
    s->pre_names[s->npre] = name;
    s->npre++;
}

void spec_add_postcondition(Spec* s, Postcondition p, const char* name) {
    if (s->npost >= MAX_POSTCONDITIONS) return;
    s->post[s->npost] = p;
    s->post_names[s->npost] = name;
    s->npost++;
}

void spec_add_invariant(Spec* s, SpecInvariant inv, const char* name) {
    if (s->ninv >= MAX_SPEC_INVARIANTS) return;
    s->inv[s->ninv] = inv;
    s->inv_names[s->ninv] = name;
    s->ninv++;
}

void spec_result_init(SpecResult* r) {
    r->npass = 0;
    r->nfail = 0;
    r->nfailures = 0;
}

bool spec_check(Spec* s, void* input, void (*func)(void*, void*), void* output) {
    printf("=== Spec Check: %s ===\n", s->name);

    bool ok = true;
    for (int i = 0; i < s->npre; i++) {
        if (!s->pre[i](input)) {
            printf("  FAIL pre: %s\n", s->pre_names[i]);
            ok = false;
        } else {
            printf("  PASS pre: %s\n", s->pre_names[i]);
        }
    }

    for (int i = 0; i < s->ninv; i++) {
        if (!s->inv[i](input)) {
            printf("  FAIL inv(before): %s\n", s->inv_names[i]);
            ok = false;
        }
    }

    func(input, output);

    for (int i = 0; i < s->npost; i++) {
        if (!s->post[i](input, output)) {
            printf("  FAIL post: %s\n", s->post_names[i]);
            ok = false;
        } else {
            printf("  PASS post: %s\n", s->post_names[i]);
        }
    }

    for (int i = 0; i < s->ninv; i++) {
        if (!s->inv[i](output)) {
            printf("  FAIL inv(after): %s\n", s->inv_names[i]);
            ok = false;
        }
    }

    printf("  Result: %s\n\n", ok ? "ALL PASS" : "SOME FAILED");
    return ok;
}

bool spec_run_and_verify(Spec* s, void* input, SafeFunc func, void* output) {
    return spec_check(s, input, func, output);
}

void spec_result_print(SpecResult* r) {
    printf("Spec result: %d pass, %d fail\n", r->npass, r->nfail);
}
