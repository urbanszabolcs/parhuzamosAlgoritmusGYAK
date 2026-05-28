#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <omp.h>

#define THREAD_NUM 4
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
    int N = data->N;
    int thread_id = data->thread_id;
    float *x = data->x;
    float *y = data->y;
    float *z = data->z;
    float *res = data->res;
    int chunk = N / THREAD_NUM;
    int local_count = data->local_count;

    int start = thread_id * chunk;
    int end = (thread_id == THREAD_NUM - 1) ? N : start + chunk;

    for (int i = start; i < end; i++)
    {
        res[i] = sqrtf(x[i] * x[i] + y[i] * y[i] + z[i] * z[i]);
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
    int local_count = 0;
    int N = data->N;
    int thread_id = data->thread_id;
    float *x = data->x;
    float *y = data->y;
    float *z = data->z;
    float *res = data->res;

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
            if (res[i] < 1.0f)
            {
                local_count++;
            }
        }
    }
    data->local_count = local_count;
    return 0;
}

void test_run(void *(*worker_fuc)(void *), const char *method_name, int N, float *x, float *y, float *z, float *res)
{
    pthread_t threads[THREAD_NUM];
    ThreadData data[THREAD_NUM];
    int shared_idx = 0;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    double start_time = omp_get_wtime();
    for (int i = 0; i < THREAD_NUM; i++)
    {
        data[i].thread_id = i;
        data[i].local_count = 0;
        data[i].N = N;
        data[i].x = x;
        data[i].y = y;
        data[i].z = z;
        data[i].res = res;
        data[i].shared_thread_index = &shared_idx;
        data[i].mutex = &mutex;

        pthread_create(&threads[i], NULL, worker_fuc, &data[i]);
    }

    for (int i = 0; i < THREAD_NUM; i++)
    {
        pthread_join(threads[i], NULL);
    }
    double end_time = omp_get_wtime();
    pthread_mutex_destroy(&mutex);

    int count = 0;
    for (int i = 0; i < THREAD_NUM; i++)
    {
        count += data[i].local_count;
    }
    printf("[%s] TALALATOK SZAMA: %d | ELTELT IDO %f masodpercben\n", method_name, count, end_time - start_time);

}

int main()
{
    int N;
    printf("KEREM ADJON MEG EGY SZAMOT!");
    if (scanf("%d", &N) != 1 || N <= 0)
    {
        printf("KEREM ADJON MEG EGY VALOS SZAMOT!");
        return 1;
    }
    float *x = (float *)malloc(N * sizeof(float));
    float *y = (float *)malloc(N * sizeof(float));
    float *z = (float *)malloc(N * sizeof(float));
    float *res = (float *)malloc(N * sizeof(float));
    if (!x || !y || !z || !res)
    {
        printf("MEMORIA HIBA, KEREM ADJON MEG EGY KISEBB SZAMOT!!!");
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