#include "barrier_linear.h"

void barrier_linear_init(barrier_linear_t *b, unsigned int wait_for) {
    b->count    = 0;
    b->wait_for = wait_for;
    pthread_mutex_init(&(b->count_lock), NULL);
    pthread_cond_init(&(b->proceed), NULL);
}

void barrier_linear_wait(barrier_linear_t *b) {
    pthread_mutex_lock(&(b->count_lock));

    b->count++;
    if (b->count == b->wait_for) {
        b->count = 0;
        pthread_cond_broadcast(&(b->proceed));
    } else {
        while (pthread_cond_wait(&(b->proceed), &(b->count_lock)) != 0)
            ;
    }

    pthread_mutex_unlock(&(b->count_lock));
}

void barrier_linear_destroy(barrier_linear_t *b) {
    pthread_cond_destroy(&(b->proceed));
    pthread_mutex_destroy(&(b->count_lock));
}
