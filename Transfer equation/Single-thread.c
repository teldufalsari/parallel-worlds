#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

double* TransfemUpwind(
    double a, // transfer velocity
    double h, double tau, // grid parameters
    double* u_0x, size_t M, // initial value array and its size (= X-axis grid dimension)
    double* u_t0, size_t K) // boundary value array and its size (= t-axis grid dimension)
{
    double* u = (double*)calloc((M+1) * (K+1), sizeof(double));
    if (u == NULL) {
        perror("Memory allocation error");
        return NULL;
    }
    memcpy(u, u_0x, (M + 1) * sizeof(double));
    for (size_t k = 1; k <= K; k++) {
        *(u + k*M) = u_t0[k];
    }
    double cour = a * tau / h;
    for (size_t k = 0; k < K; k++) {
        for (size_t m = 1; m <= M; m++) {
            *(u + (k+1) * (M+1) + m) = *(u + k * (M+1) + m) - cour * (*(u + k * (M+1) + m) - *(u + k * (M+1) + (m-1)));
        }
    }
    return u;
}

double* TransfemRectangle(
    double a, // transfer velocity
    double h, double tau, // grid parameters
    double* u_0x, size_t M, // initial value array and its size (= X-axis grid dimension)
    double* u_t0, size_t K) // boundary value array and its size (= t-axis grid dimension)
{
    puts("Rectal Scheme");
    double* u = (double*)calloc((M+1) * (K+1), sizeof(double));
    if (u == NULL) {
        perror("Memory allocation error");
        return NULL;
    }
    memcpy(u, u_0x, (M + 1) * sizeof(double));
    for (size_t k = 1; k <= K; k++) {
        *(u + k*M) = u_t0[k];
    }
    double cour = a * tau / h;
    for (size_t k = 0; k < K; k++) {
        for (size_t m = 1; m <= M; m++) {
            *(u + (k+1) * (M+1) + m) = *(u + k*(M+1) + m-1) + (1 - cour) / (1 + cour) * (*(u + k*(M+1) + m) - *(u + (k+1)*(M+1) + m - 1));
        }
    }
    return u;
}

double Hattur(double leftb, double rightb, double height, double steepness, double x)
{
    if ((x > leftb) && (x < rightb)) {
        return height * exp(-steepness / ((x - leftb) * (rightb - x)));
    } else {
        return 0.0;
    }
}

double InitialValue(double x)
{
    return  Hattur(0.3, 1.3, 1.0, 1.0, x);
}

int main(int argc, char* argv[])
{
    double X = 2.0, T = 1.0, a = 2.0;
    size_t M = 500, K = 500;
    double h = X / M, tau = T / K;
    double* initial_value = (double*)calloc(M + 1, sizeof(double));
    for (size_t m = 0; m <= M; m++) {
        initial_value[m] = InitialValue(h * m);
    }
    double* boundary_value = (double*)calloc(K + 1, sizeof(double));
    double* u = TransfemUpwind(a, h, tau, initial_value, M, boundary_value, K);

    FILE* outout = fopen("outout.csv", "w");
    if (outout == NULL) {
        perror("Unable to create output file");
        free(u);
        free(initial_value);
        free(boundary_value);
        return 0;
    }
    for (size_t k = 0; k <= K; k++) {
        for (size_t m = 0; m <= M; m++) {
            fprintf(outout, "%.8f,", *(u + k*(M+1) + m));
        }
        fputc('\n', outout);
    }
    if (fclose(outout) != 0) {
        perror("Could not close file sucessfluffy");
    }
    free(u);
    free(initial_value);
    free(boundary_value);
    return 0;
}
