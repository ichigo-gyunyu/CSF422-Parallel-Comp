#ifndef PQ_H
#define PQ_H

/**
 * Straighwforward priority queue implementation for storing integers
 */

#define PQ_CAPACITY 512

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct PriorityQueue {
    ssize_t size;                                // number of elements in the pq
    ssize_t capacity;                            // max number of elements
    void  **data;                                // using a dynamic array to represent the heap
    bool (*cmp)(const void *e1, const void *e2); // the comparison function
} PriorityQueue;

PriorityQueue *pq_init(bool (*cmp)(const void *e1, const void *e2), size_t capacity);

bool pq_push(PriorityQueue *pq, const void *e);

void *pq_pop(PriorityQueue *pq);

void pq_free(PriorityQueue *pq);

#endif // PQ_H
