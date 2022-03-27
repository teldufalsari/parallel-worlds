#define _GNU_SOURCE
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <errno.h>
#include <mpfr.h>

#define DEFAULT_PRECISION  (mpfr_prec_t)512
#define TAG_FOR_SIZES 1
#define TAG_FOR_DATA 0

size_t SerializeMPFR(mpfr_t number, char** buf_ptr)
{
    size_t msg_size = 0ul;
    FILE* msg_stream = open_memstream(buf_ptr, &msg_size); //check what it returns
    mpfr_fpif_export(msg_stream, number); // what does this function return??
    fclose(msg_stream); // check what it returns
    return msg_size;
}

void DeserializeMPFR(mpfr_t buffer, char* string, size_t length)
{
    FILE* msg_stream = fmemopen(string, length, "r");
    mpfr_fpif_import(buffer, msg_stream);
    fclose(msg_stream);
}

void SetLimits(
    unsigned long N,
    int comm_size,
    int comm_rank,
    unsigned long* lower_limit,
    unsigned long* upper_limit
    )
{
    int rem = N % comm_size;
    unsigned long N_op = N / comm_size;
    if (comm_rank <= rem) {
        *lower_limit = 1 + comm_rank * (1 + N_op);
    } else {
        *lower_limit = 1 + rem + comm_rank * N_op;
    }
    if (comm_rank < rem) {
        *upper_limit = *lower_limit + 1 + N_op;
    } else {
        *upper_limit = *lower_limit + N_op;  
    }
}

int PerformForCoreZero(int comm_size, int comm_rank, unsigned long N)
{   
    printf("Calculating Euler's number with %ld significant digits...\n", N);
    mpfr_set_default_prec(DEFAULT_PRECISION);

    //Calculate sum from 1 to upper_limit
    //## Subtask: find upper limit
    unsigned long upper_limit = 1 + N / comm_size;
    int rem = N % comm_size;
    if (rem > comm_rank) {
        upper_limit += 1;
    }
    //## Subtask: initialize variables
    mpfr_t accumulator, term;
    mpfr_init_set_ui(accumulator, 1ul, MPFR_RNDN);
    mpfr_init_set_ui(term, 1ul, MPFR_RNDN);
    //## Subtask: calculate the sum 1 + 1/(1!) + 1/(2!) +...
    for (unsigned long k = 1; k < upper_limit; k++) {
        mpfr_div_ui(term, term, k, MPFR_RNDN);
        mpfr_add(accumulator, accumulator, term, MPFR_RNDN);
    }
    int state = MPI_SUCCESS; // a variable for checking the state of MPI communication
    MPI_Status status = {}; // -//-

    // Send value required by the next core
    if (comm_rank < comm_size - 1) {
        //## Subtask: serialize the number
        char* msg_buffer = NULL;
        size_t msg_size = SerializeMPFR(term, &msg_buffer);
        //## Subtask: send serialized number to the next core
        state = MPI_Send((const void*)&msg_size, 1, MPI_UNSIGNED_LONG, comm_rank + 1, TAG_FOR_SIZES, MPI_COMM_WORLD); //send size of data buffer
        if (state != MPI_SUCCESS) {
            printf("Core %d failed to send intermediate results (size). The answer is probably incorrect.\n", comm_rank);
        }
        state = MPI_Send((const void*)msg_buffer, msg_size, MPI_CHAR, comm_rank + 1, TAG_FOR_DATA, MPI_COMM_WORLD); // send data buffer
        if (state != MPI_SUCCESS) {
            printf("Core %d failed to send intermediate results (data). The answer is probably incorrect.\n", comm_rank);
        }
        free(msg_buffer);
    }

    // Recieve other cores' results (if there are any)
    if (comm_size > 1) {
        //## Subtask: gather other cores' buffer size
        size_t* sizes = (size_t*)calloc(comm_size - 1, sizeof(size_t)); // check calloc
        for (int i = 1; i < comm_size; i++) {
            state = MPI_Recv((void*)(sizes + i - 1), 1, MPI_UNSIGNED_LONG, i, TAG_FOR_SIZES, MPI_COMM_WORLD, &status);
            if (state != MPI_SUCCESS) {
                printf("Core Zer0: failed to recieve message size from Core %d\nThe answer is probably incorrect\n");
            }
        }
        //## Subtask: allocate data buffer and recieve
        size_t total_length = 0;
        for (int i = 0; i < comm_size - 1; i++) {
            total_length += sizes[i];
        }
        char* metabuffer = (char*)calloc(total_length, sizeof(char)); // check calloc
        size_t carry = 0;
        for (int i = 1; i < comm_size; i++) {
            state = MPI_Recv((void*)(metabuffer + carry), sizes[i - 1], MPI_CHAR, i, TAG_FOR_DATA, MPI_COMM_WORLD, &status);
            if (state != MPI_SUCCESS) {
                printf("Core Zer0: failed to recieve data from Core %d\nThe answer is probably incorrect\n");
            }
            carry += sizes[i - 1];
        }
        // Reduce the answer
        carry = 0;
        for (int i = 0; i < comm_size - 1; i++) {
            DeserializeMPFR(term, metabuffer + carry, sizes[i]);
            mpfr_add(accumulator, accumulator, term, MPFR_RNDN);
            carry += sizes[i];
        }
        free(sizes);
        free(metabuffer);
    }
    // Print the answer
    puts("Core Zer0: Behold, answer:");
    mpfr_out_str(stdout, 10, 0, accumulator, MPFR_RNDN);
    mpfr_clear(accumulator);
    mpfr_clear(term);
}

int PerformForCoreN(int comm_size, int comm_rank, unsigned long N)
{
    mpfr_set_default_prec(DEFAULT_PRECISION);
    // Calculate sum from lower_limit to upper_limit
    //## Subtask: find sum limits
    unsigned long lower_limit = 0, upper_limit = 0;
    SetLimits(N, comm_size, comm_rank, &lower_limit, &upper_limit);
    //## Subtask: calculate the sum
    mpfr_t accumulator, term;
    mpfr_init_set_ui(accumulator, 0ul, MPFR_RNDN);
    mpfr_init_set_ui(term, 1ul, MPFR_RNDN);
    for (unsigned long k = lower_limit; k < upper_limit; k++) {
        mpfr_div_ui(term, term, k, MPFR_RNDN);
        mpfr_add(accumulator, accumulator, term, MPFR_RNDN);
    }

    // Recieve intermediate result from previous core
    int state = MPI_SUCCESS; // a variable for checking the state of MPI communication
    MPI_Status status = {}; // -//-
    //Subtask:: receive term size from the previous core
    size_t buf_size = 0ul;
    state = MPI_Recv((void*)&buf_size, 1, MPI_UNSIGNED_LONG, comm_rank - 1, TAG_FOR_SIZES, MPI_COMM_WORLD, &status);
    if (state != MPI_SUCCESS) {
        printf("Core %d failed to recieve intermediate results (size). The answer is probably incorrect.\n", comm_rank);
    }
    //## Subtask: receive term from previous core
    mpfr_t prev_term;
    mpfr_init2(prev_term, DEFAULT_PRECISION);
    char* buf = (char*)calloc(buf_size, sizeof(char));
    state = MPI_Recv((void*)buf, buf_size, MPI_CHAR, comm_rank - 1, TAG_FOR_DATA, MPI_COMM_WORLD, &status);
    if (state != MPI_SUCCESS) {
        printf("Core %d failed to recieve intermediate results (data). The answer is probably incorrect.\n", comm_rank);
    }
    DeserializeMPFR(prev_term, buf, buf_size);
    free(buf);

    // Apply intermediate results
    mpfr_mul(term, term, prev_term, MPFR_RNDN);
    mpfr_mul(accumulator, accumulator, prev_term, MPFR_RNDN);
    mpfr_clear(prev_term);
    // Send term to the next core if needed
    if (comm_rank < comm_size - 1) {
        //## Subtask: send size
        buf_size = SerializeMPFR(term, &buf);
        state = MPI_Send((const void*)&buf_size, 1, MPI_UNSIGNED_LONG, comm_rank + 1, TAG_FOR_SIZES, MPI_COMM_WORLD);
        if (state != MPI_SUCCESS) {
            printf("Core %d failed to send intermediate results (size). The answer is probably incorrect.\n", comm_rank);
        }
        //## Subtask: send data
        state = MPI_Send((const void*)buf, buf_size, MPI_CHAR, comm_rank + 1, TAG_FOR_DATA, MPI_COMM_WORLD);
        if (state != MPI_SUCCESS) {
            printf("Core %d failed to send intermediate results (data). The answer is probably incorrect.\n", comm_rank);
        }
        free(buf);
    }
    // Send final results
    //## Subtask: send size
    buf_size = SerializeMPFR(accumulator, &buf);
    state = MPI_Send((const void*)&buf_size, 1, MPI_UNSIGNED_LONG, 0, TAG_FOR_SIZES, MPI_COMM_WORLD);
    if (state != MPI_SUCCESS) {
        printf("Core %d failed to send final results (size). The answer is probably incorrect.\n", comm_rank);
    }
    //## Subtask: send data
    state = MPI_Send((const void*)buf, buf_size, MPI_CHAR, 0, TAG_FOR_DATA, MPI_COMM_WORLD);
    if (state != MPI_SUCCESS) {
        printf("Core %d failed to send final results (data). The answer is probably incorrect.\n", comm_rank);
    }
    free(buf);
    mpfr_clear(accumulator);
    mpfr_clear(term);
}

int main(int argc, char* argv[])
{
    // Get communicator size and rank
    int comm_size, comm_rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
    // Check if input is incorrect
    if (argc != 2) {
        if (comm_rank == 0) { // Core Zer0 exits verbosely, other cores -- silently
            printf("Usage: %s N, N is a positive integer\n");
        }
        MPI_Finalize();
        return 1;
    }
    // Convert string argument to integer
    errno = 0;
    unsigned long N = strtoul(argv[1], NULL, 10);
    if (errno != 0) {
        if (comm_rank == 0){ // Core Zer0 exits verbosely
            perror("Invalid precision");
        }
        MPI_Finalize();
        return 1;
    }
    // Calculate and print
    int state = MPI_SUCCESS; // To store exit code
    if (comm_rank == 0) {
        state = PerformForCoreZero(comm_size, comm_rank, N);
    } else { // any other core
        state = PerformForCoreN(comm_size, comm_rank, N);
    }
    MPI_Finalize();
    return state;
}
