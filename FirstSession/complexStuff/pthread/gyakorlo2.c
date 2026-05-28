#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include <pthread.h>

#define THREAD_NUMBER 4
#define CHUNK_SIZE 10000

typedef struct
{
    int thread_id;
    int N;
    float *x;
    float *y;
    float *z;
    float *res;
    int global_count;
    int local_count;
    int *shared_thread_index;

    pthread_mutex_t *mutex;
} ThreadData;

void *static_worker(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int thread_id = data->thread_id;
    int N = data->N;
    int chunk = N / THREAD_NUMBER;
    int start = thread_id * chunk;
    int end = (thread_id == THREAD_NUMBER) ? N : start + chunk;

    float *res = data->res;
    float *x = data->x;
    float *y = data->y;
    float *z = data->z;

    int local_count = data->local_count;

    for (int i = start; i < end; i++)
    {
        res[i] = sqrtf(x[i] * x[i] + y[i] * y[i] + z[i] * z[i]);
        if (res[i] < 1)
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
    float *res = data->res;
    float *x = data->x;
    float *y = data->y;
    float *z = data->z;

    while (1)
    {
        int start, end;
        pthread_mutex_lock(data->mutex);
        start = *(data->shared_thread_index);
        *(data->shared_thread_index) += CHUNK_SIZE;
        pthread_mutex_unlock(data->mutex);

        if (start > N)
            break;
        end = start + CHUNK_SIZE;
        if (end > N)
            end = N;

        for (int i = start; i < end; i++)
        {
            res[i] = sqrtf(x[i] * x[i] + y[i] * y[i] + z[i] * z[i]);
            if (res[i] < 1)
            {
                local_count++;
            }
        }
    }
    data->local_count = local_count;
    return 0;
}

void test_run(void *(worker_func)(void *), const char *method_name, int N, float *x, float *y, float *z, float *res)
{
    pthread_t threads[THREAD_NUMBER];
    ThreadData data[THREAD_NUMBER];
    int shared_idx = 0;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    double start_time = omp_get_wtime();

    for (int i = 0; i < THREAD_NUMBER; i++)
    {
        data[i].thread_id = i;
        data[i].local_count = 0;
        data[i].N = N;
        data[i].x = x;
        data[i].y = y;
        data[i].z = z;
        data[i].mutex = &mutex;
        data[i].shared_thread_index = &shared_idx;
        data[i].res = res;

        pthread_create(&threads[i], NULL, worker_func, &data[i]);
    }
    for (int i = 0; i < THREAD_NUMBER; i++)
    {
        pthread_join(threads[i], NULL);
    }
    double end_time = omp_get_wtime();
    pthread_mutex_destroy(&mutex);

    int count = 0;
    for (int i = 0; i < THREAD_NUMBER; i++)
    {
        count += data[i].local_count;
    }
    printf("[%s] Talalatok szama: %d | Futasido %f masodperc\n", method_name, count, end_time - start_time);
}

int main()
{
    int N;
    printf("Kerem adja meg a problemameretet (N): ");
    if (scanf("%d", &N) != 1 || N <= 0)
    {
        printf("HEJTELEN MERET, ADJON MEG EGY MASIK SZAMOT!!!");
        return 1;
    }
    float *x = (float *)malloc(N * sizeof(float));
    float *y = (float *)malloc(N * sizeof(float));
    float *z = (float *)malloc(N * sizeof(float));
    float *res = (float *)malloc(N * sizeof(float));

    if (!x || !y || !z)
    {
        printf("MEMORIA HIBA, KEREM ADJON MEG EGY KISEBB N SZAMOT!!!");
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
    test_run(dynamic_worker, "Dinamikus", N, x, y, z, res);

    free(x);
    free(y);
    free(z);
    free(res);

    return 0;
}