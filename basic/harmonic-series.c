#include <stdio.h>
#include <mpi.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    int comm_size, comm_rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
    if (comm_rank == 0) { // core Zer0:
        if (argc != 2) { // exit verbosely
            printf("Usage: %s N\nwhere N must be an unsigned integer", argv[0]);
            MPI_Finalize();
            return 1;
        }
        errno = 0;
        unsigned long N = strtoul(argv[1], NULL, 10); // Convert string argument to integer
        if (errno != 0) { // exit verbosely
            perror("Invalid sum limit");
            MPI_Finalize();
            return 1;
        }
        unsigned long upper_limit = 1 + N / comm_size;
        int rem = N % comm_size;
        if (rem > comm_rank) {
            upper_limit++;
        }
        double accumulator = 0.0;
        for (unsigned long k = 1; k < upper_limit; k++){
            accumulator += 1.0 / (double)k;
        }
        double buffer = 0.0;
        int state = MPI_SUCCESS;
        MPI_Status status = {};
        for (int i = 1; i < comm_size; i++){
            state = MPI_Recv((void*)&buffer, 1, MPI_DOUBLE, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (state != MPI_SUCCESS) {
                printf("Core Zer0: failed to recieve data from Core %d\nThe answer is probably incorrect\n");
            }
            accumulator += buffer;
        }
        printf("Core Zer0: Behold, answer:\nSUM(1/n) from n = 1 to n = %lu is [%lf]\n", N, accumulator);

    } else { // any other core
        if (argc != 2) { // Exit silently
            MPI_Finalize();
            return 1;
        }
        errno = 0;
        unsigned long N = strtoul(argv[1], NULL, 10); // Convert string argument to integer
        if (errno != 0) {// Exit silently
            MPI_Finalize();
            return 1;
        }
        unsigned long lower_limit = 0, upper_limit = 0;
        int rem = N % comm_size;
        unsigned long N_op = N / comm_size;
        if (comm_rank <= rem) {
            lower_limit = 1 + comm_rank * (1 + N_op);
        } else {
            lower_limit = 1 + rem + comm_rank * N_op;
        }
        if (comm_rank < rem) {
            upper_limit = lower_limit + 1 + N_op;
        } else {
            upper_limit = lower_limit + N_op;
        }
        double accumulator = 0.0;
        for (unsigned long k = lower_limit; k < upper_limit; k++) {
            accumulator += 1.0 / (double)k;
        }
        int state = MPI_Send((const void*)&accumulator, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
        if (state != MPI_SUCCESS) {
            printf("Core %d failed to send results. The answer is probably incorrect.\n", comm_rank);
        }
    }
    MPI_Finalize();
}
