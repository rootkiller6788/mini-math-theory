#include "stack.h"
#include <stdlib.h>

Stack* stack_create(int capacity) {
    Stack* s = (Stack*)malloc(sizeof(Stack));
    s->data = (int*)malloc(sizeof(int) * capacity);
    s->top = -1;
    s->capacity = capacity;
    return s;
}

void stack_free(Stack* s) {
    free(s->data);
    free(s);
}

static void stack_resize(Stack* s) {
    int new_capacity = s->capacity * 2;
    int* new_data = (int*)malloc(sizeof(int) * new_capacity);
    for (int i = 0; i < s->capacity; i++) {
        new_data[i] = s->data[i];
    }
    free(s->data);
    s->data = new_data;
    s->capacity = new_capacity;
}

void stack_push(Stack* s, int val) {
    if (s->top + 1 >= s->capacity) {
        stack_resize(s);
    }
    s->data[++(s->top)] = val;
}

int stack_pop(Stack* s) {
    return s->data[(s->top)--];
}

int stack_peek(Stack* s) {
    return s->data[s->top];
}

bool stack_is_empty(Stack* s) {
    return s->top == -1;
}

int stack_size(Stack* s) {
    return s->top + 1;
}
