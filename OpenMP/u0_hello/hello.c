#include "omp.h"
#include <stdio.h>

/*
 * "Hello, world!", but parallel:
 * threads print their numbers in reverse order
 */
int main()
{
    #pragma omp parallel
    {
        int num_threads = omp_get_num_threads();
        int id = omp_get_thread_num();
        /*
         * Each thread runs a for loop, the iterations are synchronised using a barrier
         * During each iteration only one of the threads prints its number
         */
        for (int i = num_threads - 1; i >= 0; i--) {
            #pragma omp barrier
            if (id == i)
                printf("I'm thread %d\n", id);
        }
    }
    return 0;
}
