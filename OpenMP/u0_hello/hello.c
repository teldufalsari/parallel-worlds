/*
 * Task 0: "Hello, world!", but parallel:
 * (*) threads print their numbers in reverse order
 */

#include "omp.h"
#include <stdio.h>


int main()
{
    #pragma omp parallel
    {
        int num_threads = omp_get_num_threads();
        int id = omp_get_thread_num();
        /*
         * Each thread starts a for loop, the iterations are synchronised using a barrier,
         * and the value of i is the number of the thread to print
         * during this interation
         */
        for (int i = num_threads - 1; i >= 0; i--) {
            #pragma omp barrier
            if (id == i)
                printf("I'm thread %d\n", id);
        }
    }
    return 0;
}
