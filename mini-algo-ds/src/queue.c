#include "queue.h"
#include <stdlib.h>

Queue* queue_create(int capacity) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->data = (int*)malloc(sizeof(int) * capacity);
    q->head = 0;
    q->tail = 0;
    q->size = 0;
    q->capacity = capacity;
    return q;
}

void queue_free(Queue* q) {
    free(q->data);
    free(q);
}

void queue_enqueue(Queue* q, int val) {
    q->data[q->tail] = val;
    q->tail = (q->tail + 1) % q->capacity;
    q->size++;
}

int queue_dequeue(Queue* q) {
    int val = q->data[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->size--;
    return val;
}

int queue_peek(Queue* q) {
    return q->data[q->head];
}

bool queue_is_empty(Queue* q) {
    return q->size == 0;
}

int queue_size(Queue* q) {
    return q->size;
}

Deque* deque_create(int capacity) {
    Deque* d = (Deque*)malloc(sizeof(Deque));
    d->data = (int*)malloc(sizeof(int) * capacity);
    d->front = 0;
    d->back = 0;
    d->size = 0;
    d->capacity = capacity;
    return d;
}

void deque_free(Deque* d) {
    free(d->data);
    free(d);
}

void deque_push_front(Deque* d, int val) {
    d->front = (d->front - 1 + d->capacity) % d->capacity;
    d->data[d->front] = val;
    d->size++;
}

void deque_push_back(Deque* d, int val) {
    d->data[d->back] = val;
    d->back = (d->back + 1) % d->capacity;
    d->size++;
}

int deque_pop_front(Deque* d) {
    int val = d->data[d->front];
    d->front = (d->front + 1) % d->capacity;
    d->size--;
    return val;
}

int deque_pop_back(Deque* d) {
    d->back = (d->back - 1 + d->capacity) % d->capacity;
    d->size--;
    return d->data[d->back];
}

int deque_peek_front(Deque* d) {
    return d->data[d->front];
}

int deque_peek_back(Deque* d) {
    return d->data[(d->back - 1 + d->capacity) % d->capacity];
}
