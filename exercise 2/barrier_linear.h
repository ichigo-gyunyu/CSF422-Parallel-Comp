#include <pthread.h>

typedef struct barrier_linear_t {
    unsigned int    count;
    unsigned int    wait_for;
    pthread_cond_t  proceed;
    pthread_mutex_t count_lock;
} barrier_linear_t;

// clang-format off
void barrier_linear_init    (barrier_linear_t *b, unsigned int wait_for);
void barrier_linear_wait    (barrier_linear_t *b);
void barrier_linear_destroy (barrier_linear_t *b);
// clang-format on
