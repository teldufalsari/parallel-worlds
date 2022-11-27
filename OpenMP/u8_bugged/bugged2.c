/******************************************************************************
 * ЗАДАНИЕ: bugged2.c
 * ОПИСАНИЕ:
 *   Еще одна программа на OpenMP с багом.
 *   Программа демонстрирует возможности динамического распределения нагрузки
 *   при вычислении суммы элементов арифметической прогрессии.
 ******************************************************************************/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int nthreads, i, tid;
    // Переменная типа float переполняется при вычислении такой большой суммы
    double total;

    total = 0.0;
    #pragma omp parallel
    {
        tid = omp_get_thread_num();
        if (tid == 0)
        {
            nthreads = omp_get_num_threads();
            printf("Number of threads = %d\n", nthreads);
        }
        printf("Thread %d is starting...\n", tid);

        #pragma omp barrier
	// Нужно добавить reduction, чтобы результаты потоков честно складывались
        #pragma omp for schedule(dynamic, 10) reduction(+ : total)
        for (i = 0; i < 1000000; i++) 
            total += i*1.0;

        printf ("Thread %d is done! Total= %e\n", tid, total);
    }
    printf("The total total = %e\n", total);
}
