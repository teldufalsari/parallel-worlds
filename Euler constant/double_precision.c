#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char* argv[])
{
    int comm_size, comm_rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
    
    if (comm_rank == 0) { // core Zer0
        if (argc != 2) {
            printf("Usage: %s N, N is a positive integer\n");
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
        //Calculate sum from 1 to upper_limit
        unsigned long upper_limit = 2 + N / comm_size; // Finding  upper_limit
        int rem = upper_limit % comm_size;
        if (rem > comm_size) {
            upper_limit += 1;
        }
        double accumulator = 1.0, term = 1.0; // Calculating the sum 1 + 1/(1!) + 1/(2!) +...
        for (unsigned long k = 1; k < upper_limit; k++) {
            term /= k;
            accumulator += term;
        }
        // Send value required by the next core
        int state = MPI_SUCCESS;
        if (comm_rank < comm_size - 1) {
            state = MPI_Send((const void*)&(term), 1u, MPI_DOUBLE, comm_rank + 1, 0, MPI_COMM_WORLD);
            if (state != MPI_SUCCESS) {
                printf("Core %d failed to send results. The answer is probably incorrect.\n", comm_rank);
            }
        }
        // Recieve other cores' result
        MPI_Status status = {};
        for (int i = 1; i < comm_size; i++) {
            state = MPI_Recv((void*)&(term), 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (state != MPI_SUCCESS) {
                printf("Core Zer0: failed to recieve data from Core %d\nThe answer is probably incorrect\n");
            }
            accumulator += term;
        }
        printf("Core Zer0: Behold, answer: e = %.24lf\n", accumulator);
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

        // Calculate sum from lower_limit to upper_limit
        unsigned long lower_limit = 0, upper_limit = 0; // Find exact limit values
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
        double accumulator = 0.0, term = 1.0; // Calculating the sum
        for (unsigned long k = lower_limit; k < upper_limit; k++) {
            term /= k;
            accumulator += k;
        }
        int state = MPI_SUCCESS;
        MPI_Status status = {};
        double buf = 0;
        // Receive term from the previous core
        state = MPI_Recv((void*)&buf, 1, MPI_DOUBLE, comm_rank - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (state != MPI_SUCCESS) {
            printf("Core %d failed to recieve intermediate results. The answer is probably incorrect.\n", comm_rank);
        }
        term *= buf;
        accumulator *= buf;
        // Send term to the next core if needed
        if (comm_rank < comm_size - 1) {
            state = MPI_Send((const void*)&term, 1, MPI_DOUBLE, comm_rank + 1, 0, MPI_COMM_WORLD);
            if (state != MPI_SUCCESS) {
                printf("Core %d failed to send intermediate results. The answer is probably incorrect.\n", comm_rank);
            }
        }
        // Send final results
        state = MPI_Send((const void*)&term, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        if (state != MPI_SUCCESS) {
            printf("Core %d failed to send final  results. The answer is probably incorrect.\n", comm_rank);
        }
    }
    MPI_Finalize();
}
