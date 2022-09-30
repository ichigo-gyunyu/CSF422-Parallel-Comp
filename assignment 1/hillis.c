#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef SMALL
#define N    10
#define LOGN 5
#else
#define N    1000000
#define LOGN 21
#endif

#define MAX_THREADS 49

// space to store results of every step
static int **res;

// data structure that will be passed into the pthread routine
typedef struct routine_args {
    int thread_id;
    int step;
    int num_threads;
} routine_args;

// pthread routine
void *routine(void *args) {
    int thread_id   = ((routine_args *)args)->thread_id;
    int step        = ((routine_args *)args)->step;
    int num_threads = ((routine_args *)args)->num_threads;
    int skip        = 1 << (step - 1);
    int stride      = num_threads; // simulates (static,1) scheduling

    for (int i = thread_id; i < N; i += stride) {
        if (i < skip)
            res[step][i] = res[step - 1][i];
        else
            res[step][i] = res[step - 1][i] + res[step - 1][i - skip];
    }

    free(args);

    return NULL;
}

int read_input(const char *filename, int *a, const int n) {
    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL)
        return -1;

    for (int i = 0; i < n; i++) {
        fscanf(fp, "%d", &a[i]);
    }
    return 0;
}

int main() {

    // pre-allocate space
    res = calloc(LOGN, sizeof *res);
    for (int i = 0; i < LOGN; i++) {
        res[i] = calloc(N, sizeof *res[i]);
    }

    // the initial array is in the first row
    read_input("input.txt", res[0], N);

    double elapsed_seq; // time taken with 1 thread
    double best_speedup      = 0;
    int    best_thread_count = 0;

    for (int nt = 1; nt <= MAX_THREADS; nt++) {
        struct timespec tic, toc;
        // begin timing (https://stackoverflow.com/a/15977035)
        clock_gettime(CLOCK_MONOTONIC, &tic);

        // Hillis and Steele
        for (int i = 1; i < LOGN; i++) {

            // parallelize
            pthread_t threads[nt];
            for (int k = 0; k < nt; k++) {
                routine_args *ra = malloc(sizeof *ra);
                ra->step         = i;
                ra->thread_id    = k;
                ra->num_threads  = nt;
                if (pthread_create(&threads[k], NULL, &routine, ra) != 0)
                    perror("Thread creation failed");
            }

            for (int i = 0; i < nt; i++) {
                if (pthread_join(threads[i], NULL) != 0)
                    perror("Failed to join main thread");
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &toc);
        // end timing
#ifndef SMALL
        double elapsed = toc.tv_sec - tic.tv_sec;
        elapsed += (toc.tv_nsec - tic.tv_nsec) / 1000000000.0;
        if (nt == 1) {
            elapsed_seq = elapsed;
            printf("Number of threads: %2d Time taken: %f s\n", nt, elapsed);
        } else {
            double speedup = elapsed_seq / elapsed;
            printf("Number of threads: %2d Time taken: %f s Speedup: %f\n", nt, elapsed, speedup);
            if (speedup > best_speedup) {
                best_speedup      = speedup;
                best_thread_count = nt;
            }
        }
#endif
    }

#ifdef SMALL
    // to test algorithm's correctness
    printf("Results at every stage:\n\n");
    for (int i = 0; i < LOGN; i++) {
        for (int j = 0; j < N; j++) {
            printf("%5d ", res[i][j]);
        }
        printf("\n");
    }
#else
    printf("\nBest speedup value %f using %d threads\n", best_speedup, best_thread_count);
#endif

    // free up memory
    for (int i = 0; i < LOGN; i++) {
        free(res[i]);
    }
    free(res);
}
