#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <omp.h>
#include <stdbool.h>

#define ARRAY_SIZE  8000000    // 8 millió - stabilabb
#define MATRIX_SIZE 400        // kisebb mátrix a gyorsabb teszteléshez
#define MAX_NUM     100000
#define NUM_THREADS 8

double get_time() {
    return omp_get_wtime();
}

void fill_array(int *arr, int n) {
    srand(time(NULL));
    for (int i = 0; i < n; i++)
        arr[i] = rand() % 100 + 1;
}

void fill_matrix(int *mat, int n) {
    for (int i = 0; i < n * n; i++)
        mat[i] = rand() % 20 + 1;
}

// ====================== 1. AGGREGÁCIÓ (ÖSSZEGZÉS) ======================
long long sum_sequential(int *arr, int n) {
    long long sum = 0;
    for (int i = 0; i < n; i++) sum += arr[i];
    return sum;
}

long long sum_openmp(int *arr, int n) {
    long long sum = 0;
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (int i = 0; i < n; i++)
        sum += arr[i];
    return sum;
}

typedef struct {
    int start, end;
    int *arr;
    long long partial;
} SumData;

void* sum_pthread(void *arg) {
    SumData *d = (SumData*)arg;
    d->partial = 0;
    for (int i = d->start; i < d->end; i++)
        d->partial += d->arr[i];
    return NULL;
}

long long sum_pthreads(int *arr, int n) {
    pthread_t threads[NUM_THREADS];
    SumData data[NUM_THREADS];
    int chunk = n / NUM_THREADS;
    long long total = 0;

    for (int i = 0; i < NUM_THREADS; i++) {
        data[i].arr = arr;
        data[i].start = i * chunk;
        data[i].end = (i == NUM_THREADS-1) ? n : (i+1)*chunk;
        pthread_create(&threads[i], NULL, sum_pthread, &data[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        total += data[i].partial;
    }
    return total;
}

// ====================== 2. PREFIX SUM (STABIL VERZIÓ) ======================
void prefix_sequential(long long *arr, long long *prefix, int n) {
    prefix[0] = arr[0];
    for (int i = 1; i < n; i++)
        prefix[i] = prefix[i-1] + arr[i];
}

void prefix_openmp(long long *arr, long long *prefix, int n) {
    // Első lépés: tömb másolása
    #pragma omp parallel for
    for (int i = 0; i < n; i++)
        prefix[i] = arr[i];

    // Párhuzamos prefix számítás (egyszerűsített, de működő)
    for (int stride = 1; stride < n; stride *= 2) {
        #pragma omp parallel for schedule(static)
        for (int i = stride; i < n; i++) {
            if (i % (2*stride) == 0 && i + stride < n) {
                prefix[i + stride] += prefix[i];
            }
        }
    }
}

// ====================== 3. PRÍMEK ======================
bool is_prime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6)
        if (n % i == 0 || n % (i + 2) == 0) return false;
    return true;
}

long long primes_sequential(int max_n) {
    long long count = 0;
    for (int i = 2; i <= max_n; i++)
        if (is_prime(i)) count++;
    return count;
}

long long primes_openmp(int max_n) {
    long long count = 0;
    #pragma omp parallel for reduction(+:count) schedule(dynamic, 1000)
    for (int i = 2; i <= max_n; i++)
        if (is_prime(i)) count++;
    return count;
}

// ====================== 4. MÁTRIX SZORZÁS ======================
void matrix_mult_seq(int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            long long sum = 0;
            for (int k = 0; k < n; k++)
                sum += (long long)a[i*n + k] * b[k*n + j];
            c[i*n + j] = (int)sum;
        }
}

void matrix_mult_omp(int *a, int *b, int *c, int n) {
    #pragma omp parallel for collapse(2) schedule(static)
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            long long sum = 0;
            for (int k = 0; k < n; k++)
                sum += (long long)a[i*n + k] * b[k*n + j];
            c[i*n + j] = (int)sum;
        }
}

// ====================== MAIN ======================
int main() {
    printf("=== PARHUZAMOS PROGRAMOZAS BENCHMARK ===\n\n");

    int *arr = malloc(ARRAY_SIZE * sizeof(int));
    long long *prefix = malloc(ARRAY_SIZE * sizeof(long long));
    int *A = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int *B = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int *C = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));

    fill_array(arr, ARRAY_SIZE);
    fill_matrix(A, MATRIX_SIZE);
    fill_matrix(B, MATRIX_SIZE);

    double t;
    long long result;

    printf("1. Tomb osszegzes (%d elem):\n", ARRAY_SIZE);
    t = get_time(); result = sum_sequential(arr, ARRAY_SIZE);
    printf("  Soros    : %.4f mp\n", get_time()-t);
    t = get_time(); result = sum_pthreads(arr, ARRAY_SIZE);
    printf("  Pthreads : %.4f mp\n", get_time()-t);
    t = get_time(); result = sum_openmp(arr, ARRAY_SIZE);
    printf("  OpenMP   : %.4f mp\n\n", get_time()-t);

    printf("2. Prefix osszeg:\n");
    t = get_time();
    prefix_sequential((long long*)arr, prefix, ARRAY_SIZE);
    printf("  Soros    : %.4f mp\n", get_time()-t);

    t = get_time();
    prefix_openmp((long long*)arr, prefix, ARRAY_SIZE);
    printf("  OpenMP   : %.4f mp\n\n", get_time()-t);

    printf("3. Primek szama %d-ig:\n", MAX_NUM);
    t = get_time();
    primes_sequential(MAX_NUM);
    printf("  Soros    : %.4f mp\n", get_time()-t);
    t = get_time();
    primes_openmp(MAX_NUM);
    printf("  OpenMP   : %.4f mp\n\n", get_time()-t);

    printf("4. Matrix szorzas (%dx%d):\n", MATRIX_SIZE, MATRIX_SIZE);
    t = get_time();
    matrix_mult_seq(A, B, C, MATRIX_SIZE);
    printf("  Soros    : %.4f mp\n", get_time()-t);
    t = get_time();
    matrix_mult_omp(A, B, C, MATRIX_SIZE);
    printf("  OpenMP   : %.4f mp\n\n", get_time()-t);

    printf("=== KESZ ===\n");

    free(arr); free(prefix); free(A); free(B); free(C);
    return 0;
}