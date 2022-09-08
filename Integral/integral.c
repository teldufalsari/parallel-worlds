#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>

enum {FLAG_CALCULATING, FLAG_FINISHED};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

double f(double x)
{
    return sin(1 / x);
}

double SecondDerivative(double x)
{
    return (2 * cos(1 / x) - (1 / x) * sin(1 / x)) / (x * x * x);
}

// Adjust step to local function curvature to maintain precision
double GetStep(double x, double base)
{
    return base / sqrt(fabs(SecondDerivative(x)));
}

// Trapezoid integration formula
double InegrateStep(double x, double step)
{
    return 0.5 * step * (f(x) + f(x + step));
}

struct thread_arg {
    double* resptr_; // Place where result will be saved
    double* x_; // Place where current x value is stored
    int* flag_; // Flag that indicates finishing.
    double* step_base_; // Initial step value; used to find precise local step value (see function upon)
    double* up_limit_; // Upper integral limit
};

/*
 * Perform calculations per thread.
 * Threads share one common variable, placed at [xptr] address, which is the point where the intergral is calculated at current iteration.
 * When a thread finishes its iteration, it reads the variable, finds needed step value, adds this value to the x variable
 * and performs the next iteration.
 * The first thread that reaches upper limit raises the flag variable (at [flag] address), which makes all threads stop their loops.
 * Upon termination, threads save their share of the final value to the specified variable at [resptr].
 */
void* thread_body(void* arg)
{
    double* resptr = ((struct thread_arg*)arg)->resptr_;
    double* xptr = ((struct thread_arg*)arg)->x_;
    int* flagptr = ((struct thread_arg*)arg)->flag_;
    double step_base = *(((struct thread_arg*)arg)->step_base_);
    double up_limit = *(((struct thread_arg*)arg)->up_limit_);
    double result = 0.0, step = 0.0, x = 0.0; // intermediate variables, 
    while(1)
    {
        pthread_mutex_lock(&mutex);
        if (*flagptr == FLAG_FINISHED) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        x = *xptr;
        step = GetStep(x, step_base);
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

// Converts string input into numbers
int ConvertInput(char* argv[], double* lo_limit, double* up_limit, unsigned* n_threads, double* precision)
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
    *precision = strtod(argv[4], NULL);
    if (errno != 0) {
        return 1;
    }
    return 0;
}

/*
 * Usage: './integral.exe <lower limit> <upper limit> <thread number> <precision>'
 * Calculates integral of f(x) = sin(1/x) from lower_limit to upper_limit on thread_number threads with the error not less that precision.
 */
int main(int argc, char* argv[])
{
    if (argc != 5) {
        printf("Usage: %s <lower limit> <upper limit> <thread number> <precision>", argv[0]);
        return 0;
    }
    // Convert string parameters to munbers
    unsigned n_threads = 0;
    double lo_limit = 0.0, up_limit = 0.0, precision = 0.0;
    if (ConvertInput(argv, &lo_limit, &up_limit, &n_threads, &precision) != 0) {
        puts("Invalid integration limits or thread count");
        return 1;
    }
    // Check if the input parameters are correct
    if ((up_limit <= lo_limit) || (n_threads < 1)) {
        puts("Inconsistent input parameters");
        return 1;
    }
    // Initialize the mutex
    if ((errno = pthread_mutex_init(&mutex, NULL)) != 0) {
        perror("Failed to init mutex");
        return 2;
    }
    // Allocate resources needed for calculations
    double* results = (double*)calloc(n_threads, sizeof(double));
    pthread_t* workers = (pthread_t*)calloc(n_threads, sizeof(pthread_t));
    struct thread_arg* arguments = (struct thread_arg*)calloc(n_threads, sizeof(struct thread_arg));
    // Initialize shared variables
    double x = lo_limit;
    double step_base = sqrt(12 * precision);
    int flag = FLAG_CALCULATING;
    // Load initial values to the threads and start the threads routines
    for (unsigned i = 0; i < n_threads; i++) {
        arguments[i].resptr_ = results + i;
        arguments[i].x_ = &x;
        arguments[i].flag_ = &flag;
        arguments[i].step_base_ = &step_base;
        arguments[i].up_limit_ = &up_limit;
        if ((errno = pthread_create(workers + i, NULL, &thread_body, (void*)(arguments + i))) != 0) {
            perror("pthread: failed to create thread");
            free(results);
            free(workers);
            return 1;
        }
    }
    // Wait for the threads to finish
    for (unsigned i = 0; i < n_threads; i++)
        if ((errno = pthread_join(workers[i], NULL)) != 0) {
            perror("pthread: failed to join thread");
            free(results);
            free(workers);
            return 1;
        }
    pthread_mutex_destroy(&mutex);
    // Reduce and print the answer
    double answer = 0.0;
    for (unsigned i = 0; i < n_threads; i++)
        answer += results[i];
    free(results);
    free(workers); // Whirlwinds of danger are raging around us, O'erwhelming forces of darkness assail...
    printf("The calculations say the integral is equal to %.8lf\n", answer);
    return 0;
}
