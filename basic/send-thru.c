#include <mpi.h>
#include <stdio.h>
#define MSG_SIZE 10
#define FORMAT "roger-%d"

int main(int argc, char* argv[])
{
    int commsize, my_rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Status status = {};
    if (my_rank == 0) {
        char msg[MSG_SIZE] = "";
        snprintf(msg, MSG_SIZE, FORMAT, 0);
        printf("Core Zero, initiating transmission\nCodeword = '%s'\n", msg);
        int state = MPI_Send( (void*)msg, MSG_SIZE, MPI_CHAR, my_rank + 1, 1, MPI_COMM_WORLD);
        if (state != MPI_SUCCESS) {
                printf("Core Zero: could not send successfully. Sowwy masta:(\n");
                return -1;
        }
        state = MPI_Recv((void*)msg, MSG_SIZE, MPI_CHAR, commsize - 1, 1, MPI_COMM_WORLD, &status);
        if (state != MPI_SUCCESS) {
            printf("Core Zero: could not recieve successfully. Sowwy masta:(\n");
                return -1;
        }
        printf("Core Zero: successfully recieved message '%s'\n", msg);

    } else if (my_rank == commsize - 1) {
        char msg[MSG_SIZE] = "none";
        int state = MPI_Recv((void*)msg, MSG_SIZE, MPI_CHAR, my_rank - 1, 1, MPI_COMM_WORLD, &status);
        if (state != MPI_SUCCESS) {
            printf("Core %d (finale): could not recieve successfully. Sowwy masta:(\n", my_rank);
                return -1;
        }
        printf("Core %d (finale): successfully recieved message '%s'\n", my_rank, msg);
        int code = 0;
        sscanf(msg, FORMAT, &code);
        code++;
        snprintf(msg, MSG_SIZE, FORMAT, code);
        state = MPI_Send((void*)msg, MSG_SIZE, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
        if (state != MPI_SUCCESS) {
                printf("Core %d (finale): could not send successfully. Sowwy masta:(\n", my_rank);
                return -1;
        }
    } else {
        char msg[MSG_SIZE] = "none";
        int state = MPI_Recv((void*)msg, MSG_SIZE, MPI_CHAR, my_rank - 1, 1, MPI_COMM_WORLD, &status);
        if (state != MPI_SUCCESS) {
            printf("Core %d: could not recieve successfully. Sowwy masta:(\n", my_rank);
                return -1;
        }
        printf("Core %d: successfully recieved message '%s'\n", my_rank, msg);
        int code = 0;
        sscanf(msg, FORMAT, &code);
        code++;
        snprintf(msg, MSG_SIZE, FORMAT, code);
        state = MPI_Send((void*)msg, MSG_SIZE, MPI_CHAR, my_rank + 1, 1, MPI_COMM_WORLD);
        if (state != MPI_SUCCESS) {
                printf("Core %d: could not send successfully. Sowwy masta:(\n", my_rank);
                return -1;
        }
    }
    MPI_Finalize();
}
