#include <pthread.h>

struct barrier_node {
    unsigned int    count;
    pthread_cond_t  proceed_up;
    pthread_cond_t  proceed_down;
    pthread_mutex_t count_lock;
};

typedef struct barrier_logarithmic_t {
    unsigned int         num_barriers;
    unsigned int         num_threads;
    struct barrier_node *barriers;
} barrier_logarithmic_t;

// clang-format off
int  barrier_logarithmic_init   (barrier_logarithmic_t *b, unsigned int num_threads);
void barrier_logarithmic_wait   (barrier_logarithmic_t *b, unsigned int thread_id);
void barrier_logarithmic_destroy(barrier_logarithmic_t *b);
// clang-format on
