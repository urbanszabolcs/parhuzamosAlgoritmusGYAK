#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <pthread.h>

#define THREADS_NUMBER 12    //Otthon kiprobalni magasabb szallon :3
#define CHUNK_ROWS 50

typedef struct
{
    int thread_id;
    int N;

    float *A;
    float *B;
    float *C;

    int *shared_row_index;
    pthread_mutex_t *shared_mutex;
} ThreadData;

void *static_worker(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int N = data->N;
    float *A = data->A;
    float *B = data->B;
    float *C = data->C;

    int rows_per_thread = N / THREADS_NUMBER;
    int start = data->thread_id * rows_per_thread;
    int end = (data->thread_id == THREADS_NUMBER - 1) ? N : start + rows_per_thread;

    for (int i = start; i < end; i++)
    {
        for (int j = 0; j < N; j++)
        {
            float sum = 0.0f;
            for (int k = 0; k < N; k++)
            {
                sum += A[i * N + k] * B[i * N + j];
            }
            C[i * N + j] = sum;
        }
    }

    return 0;
}

void *dynamic_worker(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int N = data->N;
    float *A = data->A;
    float *B = data->B;
    float *C = data->C;

    while (1)
    {
        int start, end;
        pthread_mutex_lock(data->shared_mutex);
        start = *(data->shared_row_index);
        *(data->shared_row_index) += CHUNK_ROWS;
        pthread_mutex_unlock(data->shared_mutex);

        if (start >= N)
            break;
        end = start + CHUNK_ROWS;
        if (end > N)
            end = N;
        for (int i = start; i < end; i++)
        {
            for (int j = 0; j < N; j++)
            {
                float sum = 0.0f;
                for (int k = 0; k < N; k++)
                {
                    sum += A[i * N + k] * B[i * N + j];
                }
                C[i * N + j] = sum;
            }
        }
    }
    return 0;
}

void run_test(void *(*worker_func)(void *), const char *method_name, int N, float *A, float *B, float *C)
{
    pthread_t threads[THREADS_NUMBER];
    ThreadData data[THREADS_NUMBER];

    int shared_row = 0;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    double start = omp_get_wtime();

    for (int i = 0; i < THREADS_NUMBER; i++)
    {
        data[i].thread_id = i;
        data[i].N = N;
        data[i].A = A;
        data[i].B = B;
        data[i].C = C;
        data[i].shared_row_index = &shared_row;
        data[i].shared_mutex = &mutex;

        pthread_create(&threads[i], NULL, worker_func, &data[i]);
    }

    for (int i = 0; i < THREADS_NUMBER; i++)
    {
        pthread_join(threads[i], NULL);
    }
    double end = omp_get_wtime();
    pthread_mutex_destroy(&mutex);

    printf("[%s] Futasido: %f masodperc\n", method_name, end - start);
}

int main()
{
    int N;
    printf("Adja meg a problemameretet (N): ");
    if (scanf("%d", &N) != 1 || N <= 0)
    {
        printf("HIBA, ADJON MEG EGY MASIK SZAMOT!!!!!");
        return 1;
    }

    float *A = (float *)malloc(N * N * sizeof(float));
    float *B = (float *)malloc(N * N * sizeof(float));
    float *C = (float *)malloc(N * N * sizeof(float));

    if (!A || !B || !C)
    {
        printf("MEMORIA HIBA");
        return 1;
    }
    srand(42);
    for (int i = 0; i < N; i++)
    {
        A[i] = ((float)rand() / RAND_MAX) * 10.0f;
        B[i] = ((float)rand() / RAND_MAX) * 10.0f;
    }

    run_test(static_worker, "Statikus futtatas", N, A, B, C);
    run_test(dynamic_worker, "Dinamikus futtatas", N, A, B, C);

    free(A);
    free(B);
    free(C);
    return 0;
}
