#include "pq.h"

// for a binary tree
#define LEFT(x)   (2 * (x) + 1)
#define RIGHT(x)  (2 * (x) + 2)
#define PARENT(x) ((x) / 2)

bool cmp_max(const int32_t e1, const int32_t e2);
bool cmp_min(const int32_t e1, const int32_t e2);
void pq_heapify(PriorityQueue *pq, size_t i);

PriorityQueue *pq_init(bool (*cmp)(const void *e1, const void *e2), size_t capacity) {
    PriorityQueue *pq = malloc(sizeof *pq);

    *pq = (PriorityQueue){
        .size     = 0,
        .capacity = capacity,
        .data     = calloc(capacity, sizeof(void *)),
        .cmp      = cmp,
    };

    return pq;
}

bool pq_push(PriorityQueue *pq, const void *e) {
    if (pq->size == pq->capacity)
        return false;

    pq->data[pq->size] = (void *)e;
    size_t i           = pq->size;
    pq->size++;

    while (i > 0 && pq->cmp(pq->data[i], pq->data[PARENT(i)])) {
        void *tmp           = pq->data[i];
        pq->data[i]         = pq->data[PARENT(i)];
        pq->data[PARENT(i)] = tmp;
        i                   = PARENT(i);
    }

    return true;
}

void *pq_pop(PriorityQueue *pq) {
    if (pq->size == 0)
        return NULL;

    void *e     = pq->data[0];
    pq->data[0] = pq->data[pq->size - 1];
    pq->size--;
    pq_heapify(pq, 0);

    return e;
}

void pq_heapify(PriorityQueue *pq, size_t i) {
    size_t li = LEFT(i), ri = RIGHT(i), next;

    if (li < pq->size && pq->cmp(pq->data[li], pq->data[i]))
        next = li;
    else
        next = i;

    if (ri < pq->size && pq->cmp(pq->data[ri], pq->data[next]))
        next = ri;

    if (next != i) {
        void *tmp      = pq->data[next];
        pq->data[next] = pq->data[i];
        pq->data[i]    = tmp;

        pq_heapify(pq, next);
    }
}

void pq_free(PriorityQueue *pq) {
    if (pq != NULL) {
        free(pq->data);
    }
    free(pq);
}
