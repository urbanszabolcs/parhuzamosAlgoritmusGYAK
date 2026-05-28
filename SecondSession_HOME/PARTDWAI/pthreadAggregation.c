#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <omp.h> //idomeres miatt

#define NUM_THREADS 4
#define n 50000000

typedef struct
{
    int thread_id;
    int start_index;
    int end_index;
    int *data;
} ThreadData;

// globális tömb a résszöszegeknek (Így elkerüljük a Mutex használatát)
long long partial_sums[NUM_THREADS];

// szálak által használt függvény
void *compute_sum(void *arg)
{
    ThreadData *td = (ThreadData *)arg;
    long long local_sum = 0;

    for (int i = td->start_index; i < td->end_index; i++)
    {
        local_sum += td->data[i];
    }
    // eredmények mentése a szál saját helyére
    partial_sums[td->thread_id] = local_sum;
    pthread_exit(NULL);
}

int main()
{
    int *data = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++)
        data[i] = 1;

    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];

    double start_time = omp_get_wtime();
    // Szállak indítása és munka kiosztása
    int chunk_size = n / NUM_THREADS;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_data[i].thread_id = i;
        thread_data[i].start_index = i * chunk_size;
        thread_data[i].end_index = (i == NUM_THREADS - 1) ? n : (i + 1) * chunk_size;
        thread_data[i].data = data;

        pthread_create(&threads[i], NULL, compute_sum, (void *)&thread_data[i]);
    }
    // Szállak bevárása
    long long total_sum = 0;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
        total_sum += partial_sums[i];
    }

    double end_time = omp_get_wtime();

    printf("PTHREAD OSSZEG: %lld\n", total_sum);
    printf("PTHREAD TIME: %f mp\n", end_time - start_time);
    free(data);
    return 0;
}