#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

typedef struct {
    int* data;
    int top;
    int capacity;
} Stack;

Stack* stack_create(int capacity);
void stack_free(Stack* s);
void stack_push(Stack* s, int val);
int stack_pop(Stack* s);
int stack_peek(Stack* s);
bool stack_is_empty(Stack* s);
int stack_size(Stack* s);

#endif
