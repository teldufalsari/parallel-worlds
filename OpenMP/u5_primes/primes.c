/*
 * This program finds number of prime numbers between 0 and N,
 * where N is the first command line argument.
 */

#include "omp.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/// Returns 1 if N is prime
int is_prime(unsigned long N)
{
    unsigned long limit = (unsigned long)round(sqrt((double)N));
    for (unsigned long i = 2; i <= limit; i++)
        if (0 == N % i)
            return 0;
    return 1;
}

/// Returns prime numbers count between 0 and N
unsigned primes_count(unsigned long N)
{
    omp_lock_t lock;
    unsigned count = 0, p_count = 0;
    omp_init_lock(&lock);
    #pragma omp parallel shared(count) firstprivate(p_count)
    {
        unsigned long i;
        #pragma omp for private(i) schedule(static)
            for (i = 2; i <= N; i++)
                if (is_prime(i))
                    p_count++;

        omp_set_lock(&lock);
        count += p_count;
        omp_unset_lock(&lock);
    }
    omp_destroy_lock(&lock);
    return count;
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        puts("Incorrect input, use \"run.sh\" script");
        return 1;
    }
    unsigned long N = strtoul(argv[1], NULL, 10);
    unsigned n_primes = primes_count(N);
    printf("There are %u prime numbers between 1 and %lu\n", n_primes, N);
    return 0;
}