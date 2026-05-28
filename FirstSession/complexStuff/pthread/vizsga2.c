#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <omp.h>

#define NUM_THREADS 4
#define CHUNK_SIZE 10000

typedef struct
{
    int thread_id;
    int N;
    float *x, *y, *z, *res;
    int global_count;
    int *shared_task_index;
    int local_count;
    pthread_mutex_t *mutex;
} ThreadData;

void *static_worker(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int tid = data->thread_id;
    int N = data->N;
    int chunk = N / NUM_THREADS;
    int start = tid * chunk;
    int end = (tid == NUM_THREADS - 1) ? N : start + chunk;
    float *res = data->res;
    float *x = data->x;
    float *y = data->y;
    float *z = data->z;

    int local_count = data->local_count;

    for (int i = start; i < end; i++)
    {
        res[i] = sqrtf(x[i] * x[i] + y[i] + y[i] + z[i] + z[i]);
        if (res[i] < 1.0f)
        {
            local_count++;
        }
    }

    data->local_count = local_count;
    return 0;
}

void *dynamic_worker(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int tid = data->thread_id;
    int N = data->N;
    int local_count = data->local_count;
    while (1)
    {
        int start, end;
        pthread_mutex_lock(data->mutex);
        start = *(data->shared_task_index);
        *(data->shared_task_index) += CHUNK_SIZE;
        pthread_mutex_unlock(data->mutex);

        if (start > N)
            break;
        end = start + CHUNK_SIZE;
        if (end > N)
        {
            end = N;
        }

        for (int i = start; i < end; i++)
        {
            data->res[i] = sqrtf(data->x[i] * data->x[i] + data->y[i] + data->y[i] + data->z[i] + data->z[i]);
            if (data->res[i] < 1.0f)
            {
                local_count++;
            }
        }
    }
    data->local_count = local_count;
    return 0;
}

void test_run(void *(*worker_func)(void *), const char *method_name, int N, float *x, float *y, float *z, float *res)
{
    pthread_t threads[NUM_THREADS];
    ThreadData data[NUM_THREADS];
    int shared_idx = 0;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    double start_time = omp_get_wtime();

    for (int i = 0; i < NUM_THREADS; i++)
    {
        data[i].thread_id = i;
        data[i].local_count = 0;
        data[i].N = N;
        data[i].x = x;
        data[i].y = y;
        data[i].z = z;
        data[i].mutex = &mutex;
        data[i].shared_task_index = &shared_idx;
        data[i].res = res;

        pthread_create(&threads[i], NULL, worker_func, &data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    double end_time = omp_get_wtime();
    pthread_mutex_destroy(&mutex);
    int count = 0;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        count += data[i].local_count;
    }
    printf("[%s] Talalatok szama: %d | Futasido: %f masodperc\n", method_name, count, end_time - start_time);
}

int main()
{
    int N;
    printf("Kerem adja meg a problemmaszamot (N): ");
    if (scanf("%d", &N) != 1 || N <= 0)
    {
        printf("Ervenytelen bemenet!!!");
        return 1;
    }

    float *x = (float *)malloc(N * sizeof(float));
    float *y = (float *)malloc(N * sizeof(float));
    float *z = (float *)malloc(N * sizeof(float));
    float *res = (float *)malloc(N * sizeof(float));

    if (!x || !y || !z || !res)
    {
        printf("MEMORIA HIBA");
        return 1;
    }

    srand(time(NULL));
    for (int i = 0; i < N; i++)
    {
        x[i] = (float)rand() / RAND_MAX;
        y[i] = (float)rand() / RAND_MAX;
        z[i] = (float)rand() / RAND_MAX;
    }

    test_run(static_worker, "Statikus kiosztas", N, x, y, z, res);
    test_run(dynamic_worker, "Dinamikus kiosztas", N, x, y, z, res);

    free(x);
    free(y);
    free(z);
    free(res);
    return 0;
}