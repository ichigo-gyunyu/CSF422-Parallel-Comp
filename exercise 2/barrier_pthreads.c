/*
 * barrier_pthreads.c
 *
 * Name: Lingesh Kumaar
 * ID: 2018B4A70857P
 *
 * Usage:
 * make
 * ./barrier_pthreads n
 *
 * Compares the performance between linear and logarithmic barriers
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "barrier_linear.h"
#include "barrier_logarithmic.h"

#define OUT           "barrier_pthreads"
#define B_LINEAR      1
#define B_LOGARITHMIC 2

static unsigned int          num_threads;
static barrier_linear_t      b_lin;
static barrier_logarithmic_t b_log;

// data which is passed as args to the thread function
typedef struct routine_args {
    int  b_type;
    uint thread_id;
} routine_args;

void usage_err() {
    printf("Usage: ./" OUT " num_threads\n");
    exit(EXIT_FAILURE);
}

// simulate some task
void long_task() {
    int i = 1000;
    while (i--) {
        int j = 1000;
        while (j--)
            ;
    }
}

void *routine(void *args) {
    int b_type = ((routine_args *)args)->b_type;
    int t_id   = ((routine_args *)args)->thread_id;

    if (b_type == B_LINEAR)
        barrier_linear_wait(&b_lin);
    else
        barrier_logarithmic_wait(&b_log, t_id);

    long_task();

    free(args);

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2)
        usage_err();

    num_threads = (int)strtoll(argv[1], NULL, 10);
    pthread_t threads[num_threads];
    printf("Number of threads: %d\n", num_threads);

    /********************************** LINEAR BARRIER **********************************/

    barrier_linear_init(&b_lin, num_threads);
    int b_type = B_LINEAR;

    struct timespec tic, toc;
    double          elapsed;
    // begin timing (https://stackoverflow.com/a/15977035)
    clock_gettime(CLOCK_MONOTONIC, &tic);

    for (uint i = 0; i < num_threads; i++) {
        routine_args *ra = malloc(sizeof *ra);
        ra->b_type       = b_type;
        ra->thread_id    = i;
        if (pthread_create(&threads[i], NULL, &routine, ra) != 0)
            perror("Failed to create thread");
    }

    for (uint i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0)
            perror("Failed to join main thread");
    }

    clock_gettime(CLOCK_MONOTONIC, &toc);
    // end timing
    elapsed = toc.tv_sec - tic.tv_sec;
    elapsed += (toc.tv_nsec - tic.tv_nsec) / 1000000000.0;
    elapsed *= 1000;

    printf("Linear Barrier:      %f ms\n", elapsed);

    barrier_linear_destroy(&b_lin);

    /********************************** LOGARITHMIC BARRIER **********************************/

    if (barrier_logarithmic_init(&b_log, num_threads) == -1) {
        printf("Could not initialize logarithmic barrier (thread count must be > 1)\n");
        exit(EXIT_FAILURE);
    }
    b_type = B_LOGARITHMIC;

    // begin timing (https://stackoverflow.com/a/15977035)
    clock_gettime(CLOCK_MONOTONIC, &tic);

    for (uint i = 0; i < num_threads; i++) {
        routine_args *ra = malloc(sizeof *ra);
        ra->b_type       = b_type;
        ra->thread_id    = i;
        if (pthread_create(&threads[i], NULL, &routine, ra) != 0)
            perror("Failed to create thread");
    }

    for (uint i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0)
            perror("Failed to join main thread");
    }

    clock_gettime(CLOCK_MONOTONIC, &toc);
    // end timing
    elapsed = toc.tv_sec - tic.tv_sec;
    elapsed += (toc.tv_nsec - tic.tv_nsec) / 1000000000.0;
    elapsed *= 1000;

    printf("Logarithmic Barrier: %f ms\n", elapsed);

    barrier_logarithmic_destroy(&b_log);

    printf("\n");
    return EXIT_SUCCESS;
}
