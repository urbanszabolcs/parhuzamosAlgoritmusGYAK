#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h> //OPENMP függvényekhez :3

#define ARRAY_SIZE 100000000

int main()
{
    //Memória foglalás
    int *array = (int *)malloc(ARRAY_SIZE * sizeof(int));
    if (array == NULL)
    {
        printf("Malloc error!!!!!!\n");
        return 1;
    }
    //Tömb feltöltés
    srand(time(NULL));
    for(int i = 0; i < ARRAY_SIZE; i++){
        array[i] = rand() % 4;    
    }

    long long total_sum = 0;

    double start_time = omp_get_wtime(); //OPENMP beépítet időmérője, pontos :3

    //OPENMP
    //A fordító automatikusan szét osztja a for ciklus iterációt a rendelkezésre álló szállak között,
    //és a végén a lokális részegységeket összeadja a total_sum változóba
    #pragma omp parralel for reduction(+:total_sum)
    for (int i = 0; i < ARRAY_SIZE; i++){
        total_sum += array[i];
    }
    double end_time = omp_get_wtime();

    printf("A teljes osszeg: %lld \n", total_sum);
    printf("Futasi ido: %f masodpercben\n", end_time - start_time);

    free(array);
    return 0;
}