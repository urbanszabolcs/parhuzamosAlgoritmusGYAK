#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include <pthread.h>

#define THREAD_NUM 4
#define CHUNK_SIZE 10000

typedef struct
{
    int thread_id;
    int N;
    float *A;
    float *x;
    float *y;

    int *shared_row_index;
    pthread_mutex_t *mutex;
} ThreadData;

void *static_worker(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int N = data->N;
    float *A = data->A;
    float *x = data->x;
    float *y = data->y;
    int thread_id = data->thread_id;
    int chunk = N / THREAD_NUM;
    int start = thread_id * chunk;
    int end = (thread_id == THREAD_NUM - 1) ? N : start + chunk;

    for (int i = start; i < end; i++)
    {
        float sum = 0.0f;
        for (int j = 0; j < N; j++)
        {
            sum += A[i * N + j] * x[j];
        }
        y[i] = sum;
    }

    return 0;
}
void *dynamic_worker(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int N = data->N;
    float *A = data->A;
    float *x = data->x;
    float *y = data->y;

    while (1)
    {
        int start, end;

        pthread_mutex_lock(data->mutex);
        start = *(data->shared_row_index);
        *(data->shared_row_index) += CHUNK_SIZE;
        pthread_mutex_unlock(data->mutex);

        if (start >= N)
            break;

        end = start + CHUNK_SIZE;
        if (end > N)
            end = N;

        for (int i = start; i < end; i++)
        {
            float sum = 0.0f;
            for (int j = 0; j < N; j++)
            {
                sum += A[i * N + j] * x[j];
            }
            y[i] = sum;
        }
    }

    return 0;
}

void run_test(void *(*worker)(void *), const char *method_name, int N, float *A, float *x, float *y)
{
    pthread_t threads[THREAD_NUM];
    ThreadData data[THREAD_NUM];
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    int shared_row = 0;
    double start_time = omp_get_wtime();

    for (int i = 0; i < THREAD_NUM; i++)
    {
        data[i].thread_id = i;
        data[i].N = N;
        data[i].A = A;
        data[i].x = x;
        data[i].y = y;
        data[i].shared_row_index = &shared_row;
        data[i].mutex = &mutex;

        pthread_create(&threads[i], NULL, worker, &data[i]);
    }

    for (int i = 0; i < THREAD_NUM; i++)
    {
        pthread_join(threads[i], NULL);
    }
    double end_time = omp_get_wtime();
    pthread_mutex_destroy(&mutex);

    printf("[%s] FUTASIDO: %f masodperc \n", method_name, end_time - start_time);
}

int main()
{
    int N;
    printf("Kerem adja meg a problemameretet (N): ");
    if (scanf("%d", &N) != 1 || N <= 0)
    {
        printf("KEREM ADJON MEG EGY RENDES SZAMOT!!");
        return 1;
    }
    float *A = (float *)malloc(N * N * sizeof(float));
    float *x = (float *)malloc(N * sizeof(float));
    float *y_stat = (float *)malloc(N * sizeof(float));
    float *y_dyn = (float *)malloc(N * sizeof(float));
    if (!A || !x || !y_stat || !y_dyn)
    {
        printf("Memoriafoglalasi hiba! Adj meg egy kisebb N-t.\n");
        return 1;
    }
    srand(time(NULL));
    for (int i = 0; i < N * N; i++)
    {
        A[i] = (float)rand() / RAND_MAX;
    }
    for (int i = 0; i < N; i++)
    {
        x[i] = (float)rand() / RAND_MAX;
    }


    run_test(static_worker, "Statikus kiosztas", N, A, x, y_stat);
    run_test(dynamic_worker, "Statikus kiosztas", N, A, x, y_dyn);



    free(A);
    free(x);
    free(y_dyn);
    free(y_stat);

    return 0;
}