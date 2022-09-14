/* 
 * Task 1: Parallel program calculating the sum of natural
 * numbers from 1 to N, where N is the command line argument
 * (*) N is a command line argument
 * (*) uses reduction() and schedule()
 */

#include "omp.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        puts("Incorrect input, use \"run.sh\" script");
        return 1;
    }
    unsigned long sum_treshold = strtoul(argv[1], NULL, 10);
    unsigned long sum = 0, i = 0;
    #pragma omp parallel for schedule(static) reduction(+:sum)
        for (i = 1; i <= sum_treshold; i++)
            sum += i;
    printf("Sum = %lu\n", sum);
    return 0;
}