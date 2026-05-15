#ifndef SPEC_H
#define SPEC_H

#include <stdbool.h>

typedef bool (*Precondition)(void* input);
typedef bool (*Postcondition)(void* input, void* output);
typedef bool (*SpecInvariant)(void* state);

#define MAX_PRECONDITIONS 8
#define MAX_POSTCONDITIONS 8
#define MAX_SPEC_INVARIANTS 8

typedef struct {
    Precondition pre[MAX_PRECONDITIONS];
    const char* pre_names[MAX_PRECONDITIONS];
    int npre;

    Postcondition post[MAX_POSTCONDITIONS];
    const char* post_names[MAX_POSTCONDITIONS];
    int npost;

    SpecInvariant inv[MAX_SPEC_INVARIANTS];
    const char* inv_names[MAX_SPEC_INVARIANTS];
    int ninv;

    const char* name;
} Spec;

void spec_init(Spec* s, const char* name);
void spec_add_precondition(Spec* s, Precondition p, const char* name);
void spec_add_postcondition(Spec* s, Postcondition p, const char* name);
void spec_add_invariant(Spec* s, SpecInvariant inv, const char* name);

typedef struct {
    int npass;
    int nfail;
    const char* failures[64];
    int nfailures;
} SpecResult;

void spec_result_init(SpecResult* r);
bool spec_check(Spec* s, void* input, void (*func)(void*, void*), void* output);
void spec_result_print(SpecResult* r);

typedef void (*SafeFunc)(void* input, void* output);
bool spec_run_and_verify(Spec* s, void* input, SafeFunc func, void* output);

#endif
