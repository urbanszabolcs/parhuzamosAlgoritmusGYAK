#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 50000000


int main()
{
    double *data = (double *)malloc(ARRAY_SIZE * sizeof(double));
    double *result_serial = (double *)malloc(ARRAY_SIZE * sizeof(double));
    double *result_parallel = (double *)malloc(ARRAY_SIZE * sizeof(double));
    if(data == NULL || result_serial == NULL ||result_parallel == NULL){
        printf("MEMORY ERROR LUL!!!!!\n");
        return 1;
    }
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        data[i] = rand() % 67;
    }

    // Soros végrehajtás
    double start_serial = omp_get_wtime();
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        // Művelet szimulálása idk
        result_serial[i] = sin(data[i]) * cos(data[i]) * sqrt(data[i]);
    }
    double end_serial = omp_get_wtime();

    // Párhuzamos végrehajtás
    double start_parallel = omp_get_wtime();
    #pragma omp parralel for
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        result_parallel[i] = sin(data[i]) * cos(data[i]) * sqrt(data[i]);
    }
    double end_parallel = omp_get_wtime();

    printf("Soros futtatas ideje mp-ben: %f\n", (end_serial - start_serial));
    printf("Parhuzamos futtatas ideje mp-ben: %f\n", (end_parallel - start_parallel));

    ///////////
    printf("Gyorsulas (speedup) %f\n", (end_serial - start_serial) / (end_parallel - start_parallel));

    free(data);
    free(result_serial);
    free(result_parallel);
    return 0;
}
