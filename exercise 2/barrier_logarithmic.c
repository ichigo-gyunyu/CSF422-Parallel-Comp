#include <stdlib.h>

#include "barrier_logarithmic.h"

// internal helper function
unsigned int next_pow_2(unsigned int n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

int barrier_logarithmic_init(barrier_logarithmic_t *b, unsigned int num_threads) {
    if (num_threads < 1)
        return -1;
    b->num_threads  = num_threads;
    b->num_barriers = next_pow_2(num_threads);
    b->barriers     = malloc(b->num_barriers * sizeof *(b->barriers));

    for (uint i = 0; i < b->num_barriers; i++) {
        b->barriers[i].count = 0;
        pthread_mutex_init(&(b->barriers[i].count_lock), NULL);
        pthread_cond_init(&(b->barriers[i].proceed_up), NULL);
        pthread_cond_init(&(b->barriers[i].proceed_down), NULL);
    }

    return 0;
}

void barrier_logarithmic_wait(barrier_logarithmic_t *b, unsigned int thread_id) {
    unsigned int n = b->num_barriers;
    int          i, base;

    // propogate left nodes up
    for (i = 2, base = 0; i <= n; i *= 2) {
        int idx = base + thread_id / i;

        pthread_mutex_lock(&(b->barriers[idx].count_lock));
        b->barriers[idx].count++;

        // left node
        if (thread_id % i == 0) {
            /**
             * When the number of threads is not a power of 2, some nodes
             * will not have a partner to pair with.
             * In that case, we simply let that node propagate up
             */
            int partner_thread_id = thread_id + i / 2;
            if (partner_thread_id < b->num_threads) {
                while (b->barriers[idx].count < 2)
                    pthread_cond_wait(&(b->barriers[idx].proceed_up), &(b->barriers[idx].count_lock));
            }

            pthread_mutex_unlock(&(b->barriers[idx].count_lock));
        }

        // right node
        else {
            if (b->barriers[idx].count == 2)
                pthread_cond_signal(&(b->barriers[idx].proceed_up));
            while (pthread_cond_wait(&(b->barriers[idx].proceed_down), &(b->barriers[idx].count_lock)) != 0)
                ;

            pthread_mutex_unlock(&(b->barriers[idx].count_lock));
            break;
        }

        base += n / i;
    }

    // call waiting right nodes down
    for (i = i / 2; i > 1; i /= 2) {
        base -= n / i;
        int idx = base + thread_id / i;

        pthread_mutex_lock(&(b->barriers[idx].count_lock));
        b->barriers[idx].count = 0;
        pthread_cond_signal(&(b->barriers[idx].proceed_down));
        pthread_mutex_unlock(&(b->barriers[idx].count_lock));
    }
}

void barrier_logarithmic_destroy(barrier_logarithmic_t *b) {
    for (uint i = 0; i < b->num_barriers; i++) {
        pthread_cond_destroy(&(b->barriers[i].proceed_down));
        pthread_cond_destroy(&(b->barriers[i].proceed_up));
        pthread_mutex_destroy(&(b->barriers[i].count_lock));
    }

    free(b->barriers);
}
