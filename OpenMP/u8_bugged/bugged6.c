/******************************************************************************
* ЗАДАНИЕ: bugged6.c
* ОПИСАНИЕ:
*   Множественные ошибки компиляции
*   Программа должнна была показывать, как можно выполнять параллельный код,
*   используя функции, но что-то пошло не так.
******************************************************************************/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define VECLEN 100

float a[VECLEN], b[VECLEN];
float sum;

float dotprod()
{
    int i, tid;
   
    // Это объявление создаёт переменную внутри параллельного участка,
    // по умолчанию делая её приватной и недоступной для reduction.
    //float sum = 0.0;

    tid = omp_get_thread_num();
    #pragma omp for reduction(+:sum)
    for (i = 0; i < VECLEN; i++)
    {
        sum += (a[i] * b[i]);
        printf("  tid= %d i=%d\n", tid, i);
    }
    return sum;
}


int main (int argc, char *argv[])
{
    int i;

    for (i = 0; i < VECLEN; i++)
        a[i] = b[i] = 1.0 * i;
    sum = 0.0;

    #pragma omp parallel
    // Судя по виду функции, имелось в виду, что она должна модифицировать
    // глобальную переменную.
    dotprod();

    printf("Sum = %f\n",sum);
}


