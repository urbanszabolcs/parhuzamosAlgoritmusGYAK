#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h> //IDŐMÉRÉSRE (is) kiváló :3

#define MAX_NUMBERS 50000000

// tömb feltöltése függvény
void fill_array(int *arr, int size)
{
    for (int i = 0; i < size; i++)
    {
        arr[i] = rand() % 100;
    }
}

int main()
{
    int *data = (int *)malloc(MAX_NUMBERS * sizeof(int));
    if (data == NULL)
    {
        printf("MEMORAFOGLALASI HIBA!!!!!44!!!!\n");
        return 1;
    }
    srand(time(NULL));
    fill_array(data, MAX_NUMBERS);

    // futtatási idő mérése
    double start_time = omp_get_wtime();

    ////////// algoritmus helye, függvény meghívása, STB, STB

    long long sum = 0;
    for (int i = 0; i < MAX_NUMBERS; i++)
    {
        sum += data[i];
    }

    double end_time = omp_get_wtime();
    printf("EREDMENY (osszeg): %lld\n", sum);
    printf("FUTTATASI IDO : %f masodperc\n", end_time - start_time);

    free(data);
    return 0;
}