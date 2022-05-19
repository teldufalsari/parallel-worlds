#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>

enum {FLAG_CALCULATING, FLAG_FINISHED};
#define PRECISION 1e-4

volatile unsigned g_counter = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

double f(double x)
{
    return sin(1 / x);
}

double GetStep(double x, double prec)
{
    (void)x;
    return prec;
}

double InegrateStep(double x, double step)
{
    return 0.5 * step * (f(x) + f(x + step));
}

struct thread_arg {
    double* resptr_;
    double* x_;
    int* flag_;
    double* precision_;
    double* up_limit_;
};

void* thread_body(void* arg)
{
    double* resptr = ((struct thread_arg*)arg)->resptr_;
    double* xptr = ((struct thread_arg*)arg)->x_;
    int* flagptr = ((struct thread_arg*)arg)->flag_;
    double precision = *(((struct thread_arg*)arg)->precision_);
    double up_limit = *(((struct thread_arg*)arg)->up_limit_);
    double result = 0.0, step = 0.0, x = 0.0;
    while(1)
    {
        pthread_mutex_lock(&mutex);
        if (*flagptr == FLAG_FINISHED) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        x = *xptr;
        step = GetStep(x, precision);
        if ((step + x) > up_limit) {
            step = up_limit - x;
            *flagptr = FLAG_FINISHED;
        }
        *xptr += step;
        pthread_mutex_unlock(&mutex);
        result += InegrateStep(x, step);
    }
    *resptr = result;
    return NULL;
}

int ConvertInput(char* argv[], double* lo_limit, double* up_limit, unsigned* n_threads)
{
    errno = 0;
    *lo_limit = strtod(argv[1], NULL);
    if (errno != 0) {
        return 1;
    }
    *up_limit = strtod(argv[2], NULL);
    if (errno != 0) {
        return 1;
    }
    *n_threads = strtoul(argv[3], NULL, 10);
    if (errno != 0) {
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        printf("Usage: %s <lower limit> <upper limit> <thread number>", argv[0]);
        return 0;
    }
    unsigned n_threads = 0;
    double lo_limit = 0.0, up_limit = 0.0, precision = PRECISION;
    if (ConvertInput(argv, &lo_limit, &up_limit, &n_threads) != 0) {
        puts("Invalid integration limits or thread count");
        return 1;
    }
    if ((up_limit <= lo_limit) || (n_threads < 1)) {
        puts("Inconsistent input parameters");
        return 1;
    }
    if ((errno = pthread_mutex_init(&mutex, NULL)) != 0) {
        perror("Failed to init mutex");
        return 2;
    }

    double* results = (double*)calloc(n_threads, sizeof(double));
    pthread_t* workers = (pthread_t*)calloc(n_threads, sizeof(pthread_t));
    struct thread_arg* arguments = (struct thread_arg*)calloc(n_threads, sizeof(struct thread_arg));
    double x = lo_limit;
    int flag = FLAG_CALCULATING;
    for (unsigned i = 0; i < n_threads; i++) {
        arguments[i].resptr_ = results + i;
        arguments[i].x_ = &x;
        arguments[i].flag_ = &flag;
        arguments[i].precision_ = &precision;
        arguments[i].up_limit_ = &up_limit;
        if ((errno = pthread_create(workers + i, NULL, &thread_body, (void*)(arguments + i))) != 0) {
            perror("pthread: failed to create thread");
            free(results);
            free(workers);
            return 1;
        }
    }
    for (unsigned i = 0; i < n_threads; i++)
        if ((errno = pthread_join(workers[i], NULL)) != 0) {
            perror("pthread: failed to join thread");
            free(results);
            free(workers);
            return 1;
        }
    pthread_mutex_destroy(&mutex);
    double answer = 0.0;
    for (unsigned i = 0; i < n_threads; i++)
        answer += results[i];
    free(results);
    free(workers); // Whirlwinds of danger are raging around us, O'erwhelming forces of darkness assail...
    printf("The calculations say the integral is equal to %lf\n", answer);
    return 0;
}