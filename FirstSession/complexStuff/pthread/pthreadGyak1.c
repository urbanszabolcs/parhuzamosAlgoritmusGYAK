#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <omp.h>

#define NUM_THREADS 4
#define CHUNK_SIZE 10000

typedef struct
{
    int thread_id;
    int N;
    float *A;
    float *B;
    int local_count;

    int *shared_task_index;
    pthread_mutex_t *shared_mutex;
} ThreadData;

void *static_worker(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int tid = data->thread_id;
    int N = data->N;

    int chunk = N / NUM_THREADS;
    int start = tid * chunk;
    int end = (tid == NUM_THREADS - 1) ? N : start + chunk;

    data->local_count = 0;
    for (int i = start; i < end; i++)
    {
        data->B[i] = sinf(data->A[i] * cosf(data->A[i]) + sqrtf(data->A[i]));
        if (data->B[i] >= 0.5f)
        {
            data->local_count++;
        }
    }
    return NULL;
}

void *dynamic_worker(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int N = data->N;

    data->local_count = 0;
    while (1)
    {
        int start, end;
        pthread_mutex_lock(data->shared_mutex);
        start = *(data->shared_task_index);
        *(data->shared_task_index) += CHUNK_SIZE;
        pthread_mutex_unlock(data->shared_mutex);

        if (start >= N)
            break;

        end = start + CHUNK_SIZE;
        if (end > N)
            end = N;

        for (int i = start; i < end; i++)
        {
            data->B[i] = sinf(data->A[i] * cosf(data->A[i]) + sqrtf(data->A[i]));
            if (data->B[i] >= 0.5f)
            {
                data->local_count++;
            }
        }
    }
    return NULL;
}

void run_test(void *(*worker_func)(void *), const char *method_name, int N, float *A, float *B)
{
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data_array[NUM_THREADS];

    int shared_index = 0;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    double start_time = omp_get_wtime();

    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_data_array[i].thread_id = i;
        thread_data_array[i].N = N;
        thread_data_array[i].A = A;
        thread_data_array[i].B = B;
        thread_data_array[i].shared_task_index = &shared_index;
        thread_data_array[i].shared_mutex = &mutex;

        pthread_create(&threads[i], NULL, worker_func, &thread_data_array[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    double end_time = omp_get_wtime();
    pthread_mutex_destroy(&mutex);

    int total_count = 0;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        total_count += thread_data_array[i].local_count;
    }
    printf("[%s] Talalatok szama: %d | Futasido: %f masodperc\n", method_name, total_count, end_time - start_time);
}

int main()
{
    int N;
    printf("Adja meg a promlemameretet (N)!!!:!");
    if (scanf("%d", &N) != 1 || N <= 0)
    {
        printf("Ervenytelen bemenet!!!");
        return 1;
    }

    float *A = (float *)malloc(N * sizeof(float));
    float *B = (float *)malloc(N * sizeof(float));

    if (!A || !B)
    {
        printf("MEMORIA HIBA");
        return 1;
    }
    srand(time(NULL));
    for (int i = 0; i < N; i++)
    {
        A[i] = ((float)rand() / RAND_MAX) * 100.0f;
    }

    run_test(static_worker, "Statikus futtatas", N, A, B);
    run_test(dynamic_worker, "Dinamikus futtatas", N, A, B);

    free(A);
    free(B);

    return 0;
}