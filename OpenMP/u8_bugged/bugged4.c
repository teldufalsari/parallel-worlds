/******************************************************************************
 * ЗАДАНИЕ: bugged4.c
 * ОПИСАНИЕ:
 *   Очень простая программа параллельных манипуляций с двумерным массивом
 *   зависимо от количества исполнителей... но с segmentation fault.
 ******************************************************************************/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define N 1048

int main (int argc, char *argv[]) 
{
    int nthreads, tid, i, j;
    /****************
     * Матрица слишком большая для стека, выделим её в куче
     ****************/
    double* a = malloc(N * N * sizeof(double));
    // a должна быть shared или firstprivate, иначе у потоков она инициализируется нулём
    #pragma omp parallel shared(nthreads) firstprivate(a) private(i, j, tid)
    {
        tid = omp_get_thread_num();
        if (tid == 0) 
        {
            nthreads = omp_get_num_threads();
            printf("Number of threads = %d\n", nthreads);
        }
        printf("Thread %d starting...\n", tid);
        for (i = 0; i < N; i++)
            for (j = 0; j < N; j++)
                *(a + N*i + j) = tid + i + j; // пришлось поменять способ адресации

        printf("Thread %d done. Last element= %f\n", tid, *(a + (N-1)*N + N-1));
    }
    free(a);
}
