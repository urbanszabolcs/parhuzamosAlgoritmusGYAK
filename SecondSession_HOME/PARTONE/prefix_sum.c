#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <omp.h>

#define N 50000000
#define NUM_THREADS 8

void prefix_sum_sequential(long long *arr, long long *prefix, int n)
{
    prefix[0] = arr[0];
    for (int i = 1; i < n; i++)
        prefix[i] = prefix[i - 1] + arr[i];
}

// PTHREADS
typedef struct
{
    int id;
    long long *arr;
    long long *prefix;
    int start;
    int end;
    long long *partial_sums;
} ThreadData;

long long *global_partial = NULL;

void *pthread_prefix_phase1(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    data->prefix[data->start] = data->arr[data->start];

    for (int i = data->start + 1; i < data->end; i++)
    {
        data->prefix[i] = data->prefix[i - 1] + data->arr[i];
    }

    // Az utolsó adat elmentése a global partialba
    global_partial[data->id] = data->prefix[data->end - 1];
    pthread_exit(NULL);
}

void prefix_sum_pthreads(long long *arr, long long *prefix, int n)
{
    pthread_t threads[NUM_THREADS];
    ThreadData data[NUM_THREADS];
    int chunk = n / NUM_THREADS;

    global_partial = malloc(NUM_THREADS * sizeof(long long));

    // Lokális prefix minden szálban
    for (int i = 0; i < NUM_THREADS; i++)
    {
        data[i].id = i;
        data[i].arr = arr;
        data[i].prefix = prefix;
        data[i].start = i * chunk;
        data[i].end = (i == NUM_THREADS - 1) ? n : (i + 1) * chunk;
        pthread_create(&threads[i], NULL, pthread_prefix_phase1, &data[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // partial sums prefixe
    for (int i = 1; i < NUM_THREADS; i++)
    {
        global_partial[i] += global_partial[i - 1];
    }

    // offset hozzáadása
    for (int i = 1; i < NUM_THREADS; i++)
    {
        int start = data[i].start;
        long long offset = global_partial[i - 1];
        for (int j = start; j < data[i].end; j++)
        {
            prefix[i] += offset;
        }
    }
    free(global_partial);
}

///////OPENMP
void prefix_sum_openmp(long long *arr, long long *prefix, int n)
{
    prefix[0] = arr[0];

#pragma omp parallel for
    for (int i = 1; i < n; i++)
    {
        prefix[i] = arr[i];
    }
    /// PARALLEL PREFIX
    for (int stride = 1; stride < n; stride *= 2)
    {
#pragma omp parallel for
        for (int i = stride; i < n; i++)
        {
            prefix[i] += prefix[i - stride];
        }
    }
}

int main()
{
    long long *arr = malloc(N * sizeof(long long));
    long long *prefix_seq = malloc(N * sizeof(long long));
    long long *prefix_pth = malloc(N * sizeof(long long));
    long long *prefix_omp = malloc(N * sizeof(long long));

    srand(time(NULL));
    for (int i = 0; i < N; i++)
    {
        arr[i] = rand() % 100;
    }
    double start, end;
    // Sequential
    start = omp_get_wtime();
    prefix_sum_sequential(arr, prefix_seq, N);
    end = omp_get_wtime();
    printf("Sequential: %.4f mp\n", end - start);
    /// PTHREADS
    start = omp_get_wtime();
    prefix_sum_pthreads(arr, prefix_pth, N);
    end = omp_get_wtime();
    printf("Pthreads  (%d szal): %.4f mp\n", NUM_THREADS, end - start);
    // OpenMP
    start = omp_get_wtime();
    prefix_sum_openmp(arr, prefix_omp, N);
    end = omp_get_wtime();
    printf("OpenMP    (%d szal): %.4f mp\n", omp_get_max_threads(), end - start);

    /// Ellenőrzés
    int correct = 1;
    for (int i = 0; i < N && i < 100; i++)
    { // csak első 100-at ellenőrzünk
        if (prefix_seq[i] != prefix_pth[i] || prefix_seq[i] != prefix_omp[i])
        {
            correct = 0;
            break;
        }
    }
    printf("Eredmeny helyes: %s\n", correct ? "IGEN" : "NEM");

    free(arr);
    free(prefix_seq);
    free(prefix_pth);
    free(prefix_omp);
    return 0;
}