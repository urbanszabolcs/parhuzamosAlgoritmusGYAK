#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS 4
#define ARRAY_SIZE 1000000

// A strúktúra amit átadunk a szálaknak
typedef struct
{
    int thread_id;
    int start_index;
    int end_index;
    int *array;
    long long partial_sum; // ide menti a szál a részeredményt
} ThreadData;

// a szállak által futtatott fügvény

void *calculate_partial_sum(void *arg)
{
    // VIssza konvertáljuk a típus nélküli mutatott a mi strukturankra
    ThreadData *data = (ThreadData *)arg;
    data->partial_sum = 0;

    // csak a rábizott tartományon bellül megy végbe (start, end)
    for (int i = data->start_index; i < data->end_index; i++)
    {
        data->partial_sum = data->array[i];
    }

    pthread_exit(NULL);
}

int main()
{
    int *array = (int *)malloc(ARRAY_SIZE * sizeof(int));

    if (array == NULL)
    {
        printf("Memorafoglalasi hiba!\n");
        return 1;
    }
    // tömb feltöltése vélletlen számokkal majd <----0 tól array size-ig. amikor befejezem
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        // a modulo (%) segít hogy ne legyenek rohadt nagy számok
        array[i] = rand() % 100;
    }

    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];

    int chunk_size = ARRAY_SIZE / NUM_THREADS;

    // Szállak elindítása (Fork)

    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_data[i].thread_id = i;
        thread_data[i].start_index = i * chunk_size;

        // az utolsó száll megkapja a maradékot is, ha nem osztható pontosan :/
        thread_data[i].end_index = (i == NUM_THREADS - 1) ? ARRAY_SIZE : (i + 1) * chunk_size;
        thread_data[i].array = array;

        pthread_create(&threads[i], NULL, calculate_partial_sum, (void *)&thread_data[i]);
    }

    long long total_sum = 0;

    // szállak bevárása és az eredmények aggregálása (Join)
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
        total_sum += thread_data[i].partial_sum;
    }

    printf("A teljes osszeg: %lld\n", total_sum);

    free(array);

    return 0;
}