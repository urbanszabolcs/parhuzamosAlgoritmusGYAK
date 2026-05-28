#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#define NUM_THREADS 4
#define CHUNK_SIZE 10000

/// Global Variables

int N;
float *x, *y, *z, *res;
int global_count = 0;

// Required for dynamic
int current_task_index = 0;
pthread_mutex_t mutex;

typedef struct
{
    int thread_id;
} ThreadData;

void *static_worker(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int thread_id = data->thread_id;
    int chunk = N / NUM_THREADS;
    int start = thread_id * chunk;
    int end = (thread_id == NUM_THREADS - 1) ? N : start + chunk;

    int local_count = 0;

    for (int i = start; i < end; i++)
    {
        res[i] = sqrtf(x[i] * x[i] + y[i] * y[i] + z[i] * z[i]);
        if (res[i] <= 1.0f)
        {
            local_count++;
        }
    }
    pthread_mutex_lock(&mutex);
    global_count += local_count;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *dynamic_worker(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int local_count = 0;

    while (1)
    {
        int start, end;
        pthread_mutex_lock(&mutex);
        start = current_task_index;
        current_task_index += CHUNK_SIZE;
        pthread_mutex_unlock(&mutex);

        if (start >= N)
            break; // Ha nincs több feladat, lépjünk ki a ciklusból

        end = start + CHUNK_SIZE;
        if (end > N)
            end = N; // Overflow ellen

        for (int i = start; i < end; i++)
        {
            res[i] = sqrtf(x[i] * x[i] + y[i] * y[i] + z[i] * z[i]);
            if (res[i] <= 1.0f)
            {
                local_count++;
            }
        }
    }

    pthread_mutex_lock(&mutex);
    global_count += local_count;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

void run_test(void *(*worker_func)(void *), const char *method_name)
{
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data_array[NUM_THREADS];

    global_count = 0;
    current_task_index = 0;
    pthread_mutex_init(&mutex, NULL);

    double startTime = omp_get_wtime();

    // szálak indítása
    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_data_array[i].thread_id = i;
        pthread_create(&threads[i], NULL, worker_func, &thread_data_array);
    }
    // szálak bevárása
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    double end_time = omp_get_wtime();
    pthread_mutex_destroy(&mutex);

    double ratio = (double)global_count / N;
    printf("[%s] Arany: %f | Futasido: %f masodperc\n", method_name, ratio, end_time - startTime);
}

int main()
{
    printf("Adja meg a promlemameretet (N): ");
    if (scanf("%d", &N) != 1 || N <= 0)
    {
        printf("Ervenytelen a bemenet!!!!!!!");
        return 1;
    }

    // Memoria foglalas
    x = (float *)malloc(N * sizeof(float));
    y = (float *)malloc(N * sizeof(float));
    z = (float *)malloc(N * sizeof(float));
    res = (float *)malloc(N * sizeof(float));

    if (!x || !y || !z || !res)
    {
        printf("MEMORIA FOGLALASI HIBA!!!");
        return 1;
    }
    /// tomb feltoltes
    srand(time(NULL));
    for (int i = 0; i < N; i++)
    {
        x[i] = (float)rand() / RAND_MAX;
        y[i] = (float)rand() / RAND_MAX;
        z[i] = (float)rand() / RAND_MAX;
    }

    run_test(static_worker, "Statikus kiosztas");
    run_test(dynamic_worker, "Dinamikus kiosztas");

    free(x);
    free(y);
    free(z);
    free(res);

    return 0;
}