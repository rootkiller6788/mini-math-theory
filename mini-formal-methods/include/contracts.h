#ifndef CONTRACTS_H
#define CONTRACTS_H

#include <stdio.h>
#include <stdlib.h>

#ifdef CONTRACT_CHECK

#define REQUIRE(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "PRECONDITION FAILED at %s:%d: %s\n\t%s\n", \
                __FILE__, __LINE__, #cond, msg); \
        abort(); \
    } \
} while(0)

#define ENSURE(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "POSTCONDITION FAILED at %s:%d: %s\n\t%s\n", \
                __FILE__, __LINE__, #cond, msg); \
        abort(); \
    } \
} while(0)

#define INVARIANT(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "INVARIANT VIOLATION at %s:%d: %s\n\t%s\n", \
                __FILE__, __LINE__, #cond, msg); \
        abort(); \
    } \
} while(0)

#else
#define REQUIRE(cond, msg)  ((void)0)
#define ENSURE(cond, msg)   ((void)0)
#define INVARIANT(cond, msg) ((void)0)
#endif

#define OLD(type, expr) ((type)(expr))

typedef struct {
    void (*func)(void*);
    bool (*pre)(void*);
    bool (*post)(void*, void*);
    bool (*inv)(void*);
} Contract;

bool contract_check(Contract* c, void* input, void* output);

#endif
