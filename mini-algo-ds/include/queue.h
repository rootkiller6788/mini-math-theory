#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

typedef struct {
    int* data;
    int head;
    int tail;
    int size;
    int capacity;
} Queue;

Queue* queue_create(int capacity);
void queue_free(Queue* q);
void queue_enqueue(Queue* q, int val);
int queue_dequeue(Queue* q);
int queue_peek(Queue* q);
bool queue_is_empty(Queue* q);
int queue_size(Queue* q);

typedef struct {
    int* data;
    int front;
    int back;
    int size;
    int capacity;
} Deque;

Deque* deque_create(int capacity);
void deque_free(Deque* d);
void deque_push_front(Deque* d, int val);
void deque_push_back(Deque* d, int val);
int deque_pop_front(Deque* d);
int deque_pop_back(Deque* d);
int deque_peek_front(Deque* d);
int deque_peek_back(Deque* d);

#endif
