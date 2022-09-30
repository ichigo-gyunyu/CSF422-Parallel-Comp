#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

double getTerm(int idx);

int main(int argc, char **argv) {
    int    terms;
    double global_sum = 0;
    if (argc >= 2)
        terms = strtol(argv[1], NULL, 10);
    else
        terms = 100000; // default number of terms

    MPI_Init(NULL, NULL);

    int world_size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    double t1 = MPI_Wtime();

    // what part of the data this process must compute
    int start_idx        = rank * (terms / world_size);
    int terms_to_compute = terms / world_size;
    if (rank < (terms % world_size)) {
        terms_to_compute++;
        start_idx += rank;
    } else {
        start_idx += (terms % world_size);
    }

    // local computation
    double local_sum = 0;
    for (int i = 0; i < terms_to_compute; i++) {
        local_sum += getTerm(start_idx + i);
    }

    // parallel reduction
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double t2 = MPI_Wtime();

    if (rank == 0) {
        printf("Result = %6f\n", global_sum);
        printf("Time Elapsed = %6fs\n", t2 - t1);
        // printf("%6f\n", t2 - t1);
    }

    MPI_Finalize();
}

double getTerm(int idx) {
    double denom = (2 * idx + 1) * ((idx & 1) ? -1 : +1); // negative if odd index
    double term  = 1 / denom;
    return term;
}
