#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

void SetLimits(
    size_t N,
    int comm_size,
    int comm_rank,
    size_t* lower_limit,
    size_t* upper_limit
    )
{
    int rem = N % comm_size;
    size_t N_op = N / comm_size;
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

double Hattur(double leftb, double rightb, double height, double steepness, double x)
{
    if ((x > leftb) && (x < rightb)) {
        return height * exp(-steepness / ((x - leftb) * (rightb - x)));
    } else {
        return 0.0;
    }
}

double InitialValue(double x)
{
    return  Hattur(0.3, 1.3, 1.0, 1.0, x);
}

// ul = length of layer + 1
// ll -- usually 1 (if the boundary value is already known) or 0
void ComputeLayer(double* u, size_t layer, size_t ll, size_t ul, double cour)
{
    for (size_t m = ll; m < ul; m++) {
        u[ul * layer + m] = u[ul * (layer-1) + m] - cour * (u[ul * (layer-1) + m] - u[ul * (layer-1) + (m-1)]);
    }
}

size_t LayerLength(int comm_size, int comm_rank, size_t N)
{
    size_t ul = 1 + N / comm_size; // upper limit
    int rem = N % comm_size;
    if (rem > comm_rank) {
        ul += 1;
    }
    return ul;
}

int PerformForCoreZero(int comm_size, int comm_rank)
{
    double X = 2.0, T = 1.0, a = 2.0;
    size_t M = 500, K = 500;
    double h = X / M, tau = T / K;
    double cour = a * tau / h;
    size_t ul = LayerLength(comm_size, comm_rank, M); // upper limit
    double* boundary_value = (double*)calloc(K + 1, sizeof(double));
    // Load boundary values here
    double* u = (double*)calloc((K+1) * ul, sizeof(double)); // check calloc
    for (size_t m = 0; m < ul; m++) {
        u[m] = InitialValue(h * m);
    }
    MPI_Request req;
    ComputeLayer(u, 1, 1, ul, cour);
    if (comm_rank < comm_size - 1) {
        MPI_Isend(u + ul*2 - 1, 1, MPI_DOUBLE, comm_rank + 1, 0, MPI_COMM_WORLD, &req);
        printf("Core 0 sent %lf to core %i\n", *(u + ul*2-1), comm_rank + 1);
    }
    for (size_t k = 2; k <= K; k++) {
        ComputeLayer(u, k, 1, ul, cour);
        if (comm_rank < comm_size - 1) {
            MPI_Wait(&req, NULL);
            MPI_Isend(u + ul * (k+1) - 1, 1, MPI_DOUBLE, comm_rank + 1, 0, MPI_COMM_WORLD, &req);
            //printf("Stage %zu, sent %lf\n", k, *(u + ul*(k+1) - 1));
        }
    }
    if (comm_rank < comm_size - 1) {
        MPI_Wait(&req, NULL);
    }
    size_t* ans_sizes = (size_t*)calloc(comm_size, sizeof(size_t));
    double** ans_pages = (double**)calloc(comm_size, sizeof(double*));
    ans_sizes[0] = ul;
    ans_pages[0] = u;
    for (int i = 1; i < comm_size; i++) {
        ans_sizes[i] = LayerLength(comm_size, i, M);
        ans_pages[i] = (double*)calloc(ans_sizes[i] * (K+1), sizeof(double));
        MPI_Recv(ans_pages[i], (K+1)*ans_sizes[i], MPI_DOUBLE, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    // Output
    FILE* outout = fopen("outmt.csv", "w");
    if (outout == NULL) {
        perror("Unable to create output file");
        free(boundary_value);
        for (int i = 0; i < comm_size; i++) {
            free(ans_pages[i]);
        }
        free(ans_pages);
        free(ans_sizes);
        return 0;
    }
    //#### debug output
    //FILE* debout = fopen("dbg.csv", "w");
    //#### /debug output

    for (size_t k = 0; k <= K; k++) {
        for (int i = 0; i < comm_size; i++) {
            //printf("@@@ i = %d\n", i);
            for (int m = (int)!!i; (size_t)m < ans_sizes[i]; m++) {
                fprintf(outout, "%.8f,", *(ans_pages[i] + k*(ans_sizes[i]) + m));
                //fprintf(debout, "[%zu][%zu],", k, m);
            }
        }
        fputc('\n', outout);
        //fputc('\n', debout);
    }
    fclose(outout);
    free(boundary_value);
    for (int i = 0; i < comm_size; i++) {
        free(ans_pages[i]);
    }
    free(ans_pages);
    free(ans_sizes);
    return 0;
}

int PerformForCoreN(int comm_size, int comm_rank)
{
    double X = 2.0, T = 1.0, a = 2.0;
    size_t M = 500, K = 500;
    double h = X / M, tau = T / K;
    double cour = a * tau / h;
    size_t ul = LayerLength(comm_size, comm_rank, M); // upper limit
    size_t ll = 0;
    for (int i = 0; i < comm_rank; i++) {
        ll += LayerLength(comm_size, i, M) - 1;
    }
    // double* boundary_value = (double*)calloc(K + 1, sizeof(double));
    // Load boundary values here
    double* u = (double*)calloc((K+1) * ul, sizeof(double)); // check calloc
    for (size_t m = 0; m < ul; m++) {
        u[m] = InitialValue(h * (ll + m));
    }
    MPI_Request req;
    MPI_Recv(u + ul, 1, MPI_DOUBLE, comm_rank - 1, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Core %i recieved %lf from core %i\n", comm_rank, *(u + ul), comm_rank - 1);
    ComputeLayer(u, 1, 1, ul, cour);
    if (comm_rank < comm_size - 1) {
        MPI_Isend(u + ul - 1, 1, MPI_DOUBLE, comm_rank + 1, 0, MPI_COMM_WORLD, &req);
        printf("Core %i sent %lf to core %i\n", comm_rank, *(u + ul - 1), comm_rank + 1);
    }
    for (size_t k = 2; k <= K; k++) {
        MPI_Recv(u + ul * k, 1, MPI_DOUBLE, comm_rank - 1, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //printf("Stage %zu, recieved %lf\n", k, *(u+ul*k));
        ComputeLayer(u, k, 1, ul, cour);
        if (comm_rank < comm_size - 1) {
            MPI_Wait(&req, NULL);
            MPI_Isend(u + ul * (k+1) - 1, 1, MPI_DOUBLE, comm_rank + 1, 0, MPI_COMM_WORLD, &req);
        }
    }
    if (comm_rank < comm_size - 1) {
        MPI_Wait(&req, NULL);
    }
    // Send the answer
    MPI_Send((const void*)u, (K+1)*ul, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    // Free resources
    free(u);
    return 0;
}

int main(int argc, char* argv[])
{
    // Get communicator size and rank
    int comm_size, comm_rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
    /* Check if input is incorrect
    if (argc != 2) {
        if (comm_rank == 0) { // Core Zer0 exits verbosely, other cores -- silently
            printf("Usage: %s N, N is a positive integer\n");
        }
        MPI_Finalize();
        return 1;
    }
    */
    // Calculate and print
    int state = MPI_SUCCESS; // To store exit code
    if (comm_rank == 0) {
        state = PerformForCoreZero(comm_size, comm_rank);
    } else { // any other core
        state = PerformForCoreN(comm_size, comm_rank);
    }
    MPI_Finalize();
    return state;
}
