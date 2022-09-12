#include "omp.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        puts("Wrong argument count!");
        return 1;
    }
    // check errno
    unsigned long sum_treshold = strtoul(argv[1], NULL, 10);
    unsigned long sum = 0, i = 0;
    #pragma omp parallel for schedule(static, 1) reduction(+:sum)
        for (i = 0; i <= sum_treshold; i++)
            sum += i;
    
    printf("Sum = %lu\n", sum);
    return 0;
}