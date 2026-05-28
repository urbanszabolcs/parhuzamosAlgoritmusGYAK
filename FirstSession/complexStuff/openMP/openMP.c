#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

int main()
{
    int N;
    printf("Adja meg a problemameretet (N): ");
    if (scanf("%d", &N) != 1 || N <= 0)
    {
        printf("Ervenytelen bemenet!!!");
        return 1;
    }

    char **arr = (char **)malloc(N * sizeof(char *));
    srand(time(NULL));

    for (int i = 0; i < N; i++)
    {
        arr[i] = (char *)malloc(2 * sizeof(char *));
        arr[i][0] = 'a' + (rand() % 26);
        arr[i][1] = '\0';
    }
    double start_seq = omp_get_wtime();
    char **seq_res = (char **)malloc(N * sizeof(char *));

    for (int i = 1; i < N; i++)
    {
        int len1 = strlen(seq_res[i - 1]);
        int len2 = strlen(arr[i]);

        seq_res[i] = (char *)malloc(len1 + len2 + 1);
        strcpy(seq_res[i], seq_res[i - 1]);
        strcat(seq_res[i], arr[i]);
    }
    double end_seq = omp_get_wtime();
    printf("Soros futasi ido = %f masodpercben\n", end_seq - start_seq);

    // Párhuzamos
    double start_par = omp_get_wtime();

    char **current = (char **)malloc(N * sizeof(char *));
    char **next = (char **)malloc(N * sizeof(char *));

#pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        current[i] = strdup(arr[i]);
    }

    // CREW PREFIX
    for (int step = 1; step < N; step *= 2)
    {
#pragma omp parallel for
        for (int i = 0; i < N; i++)
        {
            if (i >= step)
            {
                int len1 = strlen(current[i - step]);
                int len2 = strlen(current[i]);
                next[i] = (char *)malloc(len1 + len2 + 1);

                strcpy(next[i], current[i - step]);
                strcpy(next[i], current[i]);
            }
            else
            {
                next[i] = strdup(current[i]);
            }
        }
#pragma omp parallel for
        for (int i = 0; i < N; i++)
        {
            free(current[i]);
            current[i] = next[i];
        }
    }

    double end_par = omp_get_wtime();
    printf("Parhuzamos futasi ido: %f masodpercben\n ", end_par - start_par);

    for (int i = 0; i < N; i++)
    {
        free(arr[i]);
        free(seq_res[i]);
        free(current[i]);
    }
    free(arr);
    free(seq_res);
    free(current);
    free(next);

    return 1;
}
