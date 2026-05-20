#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <time.h>

<<<<<<< HEAD
#define ARRAY_SIZE 10000000
=======
#define ARRAY_SIZE 50000000
>>>>>>> 3128f731265ab887ba0a8fe3b074eeac629cd32e

int main()
{
    double *data = (double *)malloc(ARRAY_SIZE * sizeof(int));
    double *result_serial = (double *)malloc(ARRAY_SIZE * sizeof(int));
    double *result_parallel = (double *)malloc(ARRAY_SIZE * sizeof(int));

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
<<<<<<< HEAD
        result_serial[i] = sin(data[i]) * cos(data[i]);
=======
        result_serial[i] = sin(data[i]) * cos(data[i]) * sqrt(data[i]);
>>>>>>> 3128f731265ab887ba0a8fe3b074eeac629cd32e
    }
    double end_serial = omp_get_wtime();

    // Párhuzamos végrehajtás
    double start_parallel = omp_get_wtime();
    #pragma omp parralel for
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
<<<<<<< HEAD
        result_parallel[i] = sin(data[i]) * cos(data[i]);
=======
        result_parallel[i] = sin(data[i]) * cos(data[i]) * sqrt(data[i]);
>>>>>>> 3128f731265ab887ba0a8fe3b074eeac629cd32e
    }
    double end_parallel = omp_get_wtime();

    printf("Soros futtatas ideje mp-ben: %f", (end_serial - start_serial));
    printf("Parhuzamos futtatas ideje mp-ben: %f", (end_parallel - start_parallel));

    ///////////
    printf("Gyorsulas (speedup) %f", (end_serial - start_serial) / (end_parallel - start_parallel));

    free(data);
    free(result_serial);
    free(result_parallel);
    return 0;
<<<<<<< HEAD
}
=======
}
>>>>>>> 3128f731265ab887ba0a8fe3b074eeac629cd32e
