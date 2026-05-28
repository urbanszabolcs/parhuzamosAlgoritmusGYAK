#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main()
{
    int n = 50000000;
    int *data = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++)
        data[i] = 1;

    long long sum = 0;

    double start_time = omp_get_wtime();

#pragma omp parallel for reduction(+ : sum)
    for (int i = 0; i < n; i++)
    {
        sum += data[i];
    }

    double end_time = omp_get_wtime();

    printf("OpenMP Osszeg: %lld\n", sum);
    printf("Ido: %f mp\n", end_time - start_time);

    free(data);
    return 0;
}