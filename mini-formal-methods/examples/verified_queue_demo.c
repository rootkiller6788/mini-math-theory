/* CONTRACT_CHECK defined via Makefile CFLAGS */
#include "contracts.h"
#include "invariants.h"
#include <stdio.h>

#define QUEUE_CAP 8

typedef struct {
    int buf[QUEUE_CAP];
    int head, tail, size;
} Queue;

bool queue_inv_bounds(void* state) {
    Queue* q = (Queue*)state;
    return q->size >= 0 && q->size <= QUEUE_CAP &&
           q->head >= 0 && q->head < QUEUE_CAP &&
           q->tail >= 0 && q->tail < QUEUE_CAP;
}

bool queue_inv_size_consistency(void* state) {
    Queue* q = (Queue*)state;
    int computed = (q->tail - q->head + QUEUE_CAP) % QUEUE_CAP;
    return computed == q->size;
}

void queue_init(Queue* q) {
    q->head = 0;
    q->tail = 0;
    q->size = 0;
    INVARIANT(queue_inv_bounds(q), "bounds ok after init");
}

bool queue_is_empty(Queue* q) { return q->size == 0; }
bool queue_is_full(Queue* q)  { return q->size == QUEUE_CAP; }

void queue_enqueue(Queue* q, int val) {
    REQUIRE(!queue_is_full(q), "queue not full for enqueue");
    INVARIANT(queue_inv_bounds(q), "bounds invariant before enqueue");

    q->buf[q->tail] = val;
    q->tail = (q->tail + 1) % QUEUE_CAP;
    q->size++;

    ENSURE(!queue_is_empty(q), "queue not empty after enqueue");
    ENSURE(q->buf[(q->tail - 1 + QUEUE_CAP) % QUEUE_CAP] == val, "enqueued value stored");
    INVARIANT(queue_inv_bounds(q), "bounds invariant after enqueue");
}

int queue_dequeue(Queue* q) {
    REQUIRE(!queue_is_empty(q), "queue not empty for dequeue");
    INVARIANT(queue_inv_bounds(q), "bounds invariant before dequeue");

    int val = q->buf[q->head];
    q->head = (q->head + 1) % QUEUE_CAP;
    q->size--;

    ENSURE(!queue_is_full(q), "queue not full after dequeue");
    INVARIANT(queue_inv_bounds(q), "bounds invariant after dequeue");
    return val;
}

int main(void) {
    printf("====== Verified Queue Demo ======\n\n");

    Queue q;
    queue_init(&q);

    printf("--- 1. Enqueue/dequeue with contracts ---\n");
    for (int i = 1; i <= 5; i++) {
        queue_enqueue(&q, i * 10);
        printf("Enqueued %d, size=%d\n", i * 10, q.size);
    }

    while (!queue_is_empty(&q)) {
        int val = queue_dequeue(&q);
        printf("Dequeued %d, size=%d\n", val, q.size);
    }
    printf("\n");

    printf("--- 2. Loop Invariant Verification ---\n");
    queue_init(&q);
    for (int i = 1; i <= 6; i++)
        queue_enqueue(&q, i);

    LoopInvariant inv_size, inv_bounds;
    loop_inv_init(&inv_size, "size >= 0", queue_inv_bounds);
    loop_inv_init(&inv_bounds, "bounds invariant", queue_inv_bounds);

    printf("Before loop:\n");
    loop_inv_establish(&inv_bounds, &q);

    for (int i = 0; i < 3; i++) {
        queue_dequeue(&q);
        printf("  Iter %d: ", i);
        if (loop_inv_maintain(&inv_bounds, &q))
            printf("invariant holds, size=%d\n", q.size);
    }

    loop_inv_report(&inv_bounds);
    printf("\n");

    printf("--- 3. Invariant: size consistency ---\n");
    queue_init(&q);
    LoopInvariant inv_cons;
    loop_inv_init(&inv_cons, "size = (tail-head) mod CAP", queue_inv_size_consistency);

    loop_inv_establish(&inv_cons, &q);
    for (int i = 0; i < 4; i++) {
        queue_enqueue(&q, i);
        loop_inv_maintain(&inv_cons, &q);
    }
    for (int i = 0; i < 2; i++) {
        queue_dequeue(&q);
        loop_inv_maintain(&inv_cons, &q);
    }
    loop_inv_report(&inv_cons);

    return 0;
}
