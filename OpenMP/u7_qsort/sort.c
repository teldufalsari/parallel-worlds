#include <omp.h>
#include <stddef.h>

void swap(int* lhs, int* rhs)
{
    int tmp = *lhs;
    *lhs = *rhs;
    *rhs = tmp;
}

void qsort_iter(int* a, size_t begin, size_t end)
{
    size_t left = begin, right = end;
    int pivot = a[(begin + end + 1) / 2];
    do {
        while(a[left] < pivot)
            left++;
        while(a[right] > pivot)
            right--;
        if (left <= right) {
            swap(a + left++, a + right--);
        }
    } while (left <= right);

    if (begin < right) {
        #pragma omp task
	{
            qsort_iter(a, begin, right);
        }
    }

    if (left < end) {
        #pragma omp task
	{
            qsort_iter(a, left, end);
        }
    }
}

void parallel_qsort(int* base, size_t nmemb)
{
    if (nmemb <= 1) {
        return;
    }
    #pragma omp parallel
    {
        #pragma omp single nowait
	{
            qsort_iter(base, 0, nmemb - 1);
        }
    }
}
