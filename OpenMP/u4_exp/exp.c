/**
 * This program calculates the following series:
 * e_N = 1 + 1/(1!) + 1/(2!) + ... + 1/(N!),
 * where N is the first command line argument.
 */
#include "omp.h"
#include <stdio.h>
#include <stdlib.h>

double calculate_exp(unsigned long N)
{
    double* sums;
    double* terms;
    unsigned long k = 0;
    double term = 1.0, accumulator = 0.0;
    int thread_count = 0;
    #pragma omp parallel shared(sums, terms, thread_count) firstprivate(term, accumulator)
    {
        // Create buffers to save intermediate data
        #pragma omp single
        {
            thread_count = omp_get_num_threads();
            sums = calloc(thread_count, sizeof(double));
            terms = calloc(thread_count, sizeof(double));
        }
        /*
         * k_i is a value the i-th thread starts its loop from.
         * The thread calculates the following sum:
         * S_i = 1/k_i + 1/(k_i *(k_i+1)) + ... + 1/(j! / (k_i - 1)!) + ... +  1 / [ (k_{i+1})! / (k_i - 1)! ]
         * To obtain the sum of the terms from k_i to k_{i+1} we need to divide S_i by (k_i - 1)!,
         * which equals the value stored in the `term' variable of the previous
         * thread (or by 1 in case i = 0).
         * This reduction is done after the parallel section.
         */
        #pragma omp for private(k) schedule(static)
            for (k = 1; k < N; k++) {
                term /= k;
                accumulator += term;
            }
        int id = omp_get_thread_num();
        sums[id] = accumulator;
        terms[id] = term;
    }
    // Reduce
    double exp = 1.0 + sums[0];
    for (int i = 1; i < thread_count; i++) {
        sums[i] *= terms[i - 1];
        terms[i] *= terms[i - 1];
        exp += sums[i];
    }
    free(sums);
    free(terms);
    return exp;
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        puts("Incorrect input, use \"run.sh\" script");
        return 1;
    }
    unsigned long sum_treshold = strtoul(argv[1], NULL, 10);
    double exp = calculate_exp(sum_treshold);
    printf("e_{%lu} = %.20lf\n", sum_treshold, exp);
    return 0;
}