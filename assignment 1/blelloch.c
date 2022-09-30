#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SMALL
#define N    10
#define LOGN 5
#else
#define N    1000000
#define LOGN 21
#endif

#define MAX_THREADS 49

static int *res; // to store results of scan algorithm
static int  n;   // power of 2 that is nearest to N

// data structure that will be passed into the pthread routine
typedef struct routine_args {
    int thread_id;
    int step;
    int num_threads;
} routine_args;

// pthread routines
void *routine_upsweep(void *args) {
    int thread_id   = ((routine_args *)args)->thread_id;
    int step        = ((routine_args *)args)->step;
    int num_threads = ((routine_args *)args)->num_threads;
    int skip        = 1 << (step - 1);
    int stride      = 2 * skip * num_threads; // simulates (static,1) scheduling

    for (int j = 2 * skip - 1 + (2 * skip * thread_id); j < n; j += stride) {
        res[j] += res[j - skip];
    }

    free(args);

    return NULL;
}

void *routine_downsweep(void *args) {
    int thread_id   = ((routine_args *)args)->thread_id;
    int step        = ((routine_args *)args)->step;
    int num_threads = ((routine_args *)args)->num_threads;
    int skip        = 1 << (step - 1);
    int stride      = 2 * skip * num_threads; // simulates (static,1) scheduling
    int upto        = 2 * skip - 1;

    for (int j = n - 1 - (2 * skip * thread_id); j >= upto; j -= stride) {
        int tmp = res[j];
        res[j] += res[j - skip];
        res[j - skip] = tmp;
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

    n = 1 << (LOGN - 1); // nearest power of 2

    // pre-allocate space to store results of every step
    // extra spaces is padded with 0s
    int *arr = calloc(n, sizeof *arr);
    res      = calloc(n, sizeof *res);

    // the initial array is in the first row
    read_input("input.txt", arr, N);

#ifdef SMALL
    printf("Original Array:\n\n");
    for (int i = 0; i < n; i++) {
        printf("%5d ", arr[i]);
    }
    printf("\n\n");
#endif

    double elapsed_seq; // time taken with 1 thread
    double best_speedup      = 0;
    int    best_thread_count = 0;

    for (int nt = 1; nt <= MAX_THREADS; nt++) {
        memcpy(res, arr, n * sizeof *arr);
        struct timespec tic, toc;
        // begin timing (https://stackoverflow.com/a/15977035)
        clock_gettime(CLOCK_MONOTONIC, &tic);

        // Blelloch up-sweep
        for (int i = 1; i < LOGN; i++) {
            // parallelize
            pthread_t threads[nt];
            for (int k = 0; k < nt; k++) {
                routine_args *ra = malloc(sizeof *ra);
                ra->step         = i;
                ra->thread_id    = k;
                ra->num_threads  = nt;
                if (pthread_create(&threads[k], NULL, &routine_upsweep, ra) != 0)
                    perror("Thread creation failed");
            }

            for (int i = 0; i < nt; i++) {
                if (pthread_join(threads[i], NULL) != 0)
                    perror("Failed to join main thread");
            }
        }

        res[n - 1] = 0;

        // Blelloch down-sweep
        for (int i = LOGN - 1; i >= 1; i--) {
            // parallelize
            pthread_t threads[nt];
            for (int k = 0; k < nt; k++) {
                routine_args *ra = malloc(sizeof *ra);
                ra->step         = i;
                ra->thread_id    = k;
                ra->num_threads  = nt;
                if (pthread_create(&threads[k], NULL, &routine_downsweep, ra) != 0)
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
            printf("Number of threads: %2d. Time taken: %f s\n", nt, elapsed);
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
    printf("After Blelloch scan:\n\n");
    for (int i = 0; i < n; i++) {
        printf("%5d ", res[i]);
    }
    printf("\n");
#else
    printf("\nBest speedup value %f using %d threads\n", best_speedup, best_thread_count);
#endif

    free(res);
}
