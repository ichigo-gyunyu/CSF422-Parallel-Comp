/*
 * pthreads.c
 *
 * Name: Lingesh Kumaar
 * ID: 2018B4A70857P
 *
 * Usage:
 * make
 * ./pthreads x n
 *
 * Computes e^x from its Taylor series using pthreads
 *
 */

#include <float.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>

// how many terms in the Taylor series to compute
#define DEGREE 100

typedef long double ld;

typedef struct thread_param {
    uint to_calculate;
    ld starting_num;
    ld starting_denom;

    ld ending_term;
    ld partial_sum;
} thread_param;

void usageError();

// thread function
void *term_runner(void *arg) {
    thread_param *tptr = (thread_param *)arg;

    uint m = tptr->to_calculate;
    ld x   = tptr->starting_num;
    ld d   = tptr->starting_denom;

    ld term     = x / d;
    ld localsum = term;
    for (uint i = 1; i < m; i++) {
        d++;
        term = term * (x / d);
        localsum += term;
    }

    tptr->partial_sum = localsum;
    tptr->ending_term = term;
    pthread_exit(tptr);
}

int main(int argc, char **argv) {
    if (argc != 3)
        usageError();

    ld x   = (ld)strtold(argv[1], (char **)NULL);
    uint n = (uint)strtol(argv[2], (char **)NULL, 10);

    // setup the pthreads
    pthread_t tids[n];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    uint extra = DEGREE % n;
    ld denom   = 1;
    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);

    // compute partial sum
    for (uint i = 0; i < n; i++) {
        thread_param *tp = malloc(sizeof *tp);
        tp->to_calculate = DEGREE / n;
        if (i < extra)
            tp->to_calculate++;
        tp->starting_num   = x;
        tp->starting_denom = denom;
        denom += tp->to_calculate;
        pthread_create(&tids[i], &attr, term_runner, tp);
    }

    // combine the partial sums
    ld ans      = 1;
    ld curr_end = 1;
    for (uint i = 0; i < n; i++) {
        void *tp;
        pthread_join(tids[i], &tp);
        thread_param *t = (thread_param *)tp;
        ans += curr_end * t->partial_sum;
        curr_end = t->ending_term * curr_end;
        free(tp);
    }

#ifndef README
    printf("e^(%Lf) = %.20Lf\n", x, ans);
#endif
    gettimeofday(&tv2, NULL);
    printf("Number of threads: %2d. Total time = %f seconds\n", n,
           (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 +
               (double)(tv2.tv_sec - tv1.tv_sec));

    return EXIT_SUCCESS;
}

void usageError() {
    fprintf(stderr, "Usage: %s <x> <num_processes>\n", "./pthreads");
    exit(EXIT_FAILURE);
}
