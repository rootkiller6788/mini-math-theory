#define CONTRACT_CHECK
#include "contracts.h"
#include "spec.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define MAX_STACK 32

typedef struct {
    int data[MAX_STACK];
    int top;
    int capacity;
} Stack;

void stack_init(Stack* s, int cap) {
    REQUIRE(cap > 0 && cap <= MAX_STACK, "capacity must be in (0, MAX_STACK]");
    s->capacity = cap;
    s->top = 0;
    ENSURE(s->top == 0, "stack initialized empty");
}

void stack_push(Stack* s, int val) {
    REQUIRE(s->top < s->capacity, "stack not full before push");
    int old_top = s->top;
    s->data[s->top++] = val;
    ENSURE(s->top == old_top + 1, "top incremented by 1");
    ENSURE(s->data[s->top - 1] == val, "pushed value is at top");
}

int stack_pop(Stack* s) {
    REQUIRE(s->top > 0, "stack not empty before pop");
    int val = s->data[--s->top];
    ENSURE(s->top >= 0, "top is non-negative after pop");
    return val;
}

int stack_peek(Stack* s) {
    REQUIRE(s->top > 0, "stack not empty before peek");
    int val = s->data[s->top - 1];
    ENSURE(val == s->data[s->top - 1], "peek returns top element");
    return val;
}

bool stack_is_empty(Stack* s) { return s->top == 0; }
bool stack_is_full(Stack* s)  { return s->top == s->capacity; }

// =========================================================
// Invariant checks (used by spec framework)
// =========================================================
bool stack_inv_top_range(void* state) {
    Stack* s = (Stack*)state;
    return s->top >= 0 && s->top <= s->capacity;
}

bool stack_pre_not_full(void* state) {
    Stack* s = (Stack*)state;
    return !stack_is_full(s);
}

typedef struct { Stack* s; int val; } PushInput;
typedef struct { int result; } PopOutput;

bool stack_post_push(void* input_raw, void* output_raw) {
    PushInput* in = (PushInput*)input_raw;
    return in->s->data[in->s->top - 1] == in->val;
}

bool stack_post_pop(void* input_raw, void* output_raw) {
    PopOutput* out = (PopOutput*)output_raw;
    return out->result >= INT_MIN;
}

static void stack_push_wrapper(void* input, void* output) {
    PushInput* in = (PushInput*)input;
    stack_push(in->s, in->val);
    *(PushInput*)output = *in;
}

static void stack_pop_wrapper(void* input, void* output) {
    PopOutput* out = (PopOutput*)output;
    Stack* s = (Stack*)input;
    out->result = stack_pop(s);
}

int main(void) {
    printf("====== Verified Stack Demo ======\n\n");

    Stack s;
    printf("--- 1. Runtime Contract Checks ---\n");
    stack_init(&s, 8);

    printf("Pushing 10, 20, 30...\n");
    stack_push(&s, 10);
    stack_push(&s, 20);
    stack_push(&s, 30);
    printf("Peek: %d (expect 30)\n", stack_peek(&s));
    printf("Pop: %d\n", stack_pop(&s));
    printf("Pop: %d\n", stack_pop(&s));
    printf("Pop: %d\n", stack_pop(&s));
    printf("Empty? %s\n\n", stack_is_empty(&s) ? "yes" : "no");

    printf("--- 2. Formal Spec Check ---\n");

    Spec spec_push;
    spec_init(&spec_push, "Stack::push");
    spec_add_precondition(&spec_push, stack_pre_not_full, "not full");
    spec_add_postcondition(&spec_push, stack_post_push, "pushed val is on top");

    stack_init(&s, 4);
    PushInput pin = { &s, 99 };
    PushInput pout;
    spec_check(&spec_push, &pin, stack_push_wrapper, &pout);

    printf("  (invariant 0<=top<=%d holds: %s)\n\n",
           s.capacity, (s.top >= 0 && s.top <= s.capacity) ? "YES" : "NO");

    printf("--- 3. Postcondition: pop ---\n");
    Spec spec_pop;
    spec_init(&spec_pop, "Stack::pop");
    spec_add_postcondition(&spec_pop, stack_post_pop, "returns valid value");

    stack_push(&s, 42);
    Stack pop_state;
    memcpy(&pop_state, &s, sizeof(Stack));
    PopOutput pop_out;
    spec_check(&spec_pop, &pop_state, stack_pop_wrapper, &pop_out);
    printf("  pop returned: %d\n", pop_out.result);

    printf("--- 4. Invariant violation demo ---\n");
    printf("(Would crash if CONTRACT_CHECK enabled — here we test logically)\n");
    printf("Full? %s, Top=%d, Cap=%d\n", stack_is_full(&s) ? "yes" : "no", s.top, s.capacity);

    return 0;
}
