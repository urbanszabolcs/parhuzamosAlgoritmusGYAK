#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

#define THREADS 4

typedef struct
{
    int current_number;
    int max_nummber;
    int total_primes;
    pthread_mutex_t mutex;
} SharedData;

// prímtesztelő függvény
bool is_prime(int n)
{
    if (n <= 1)
        return false;
    for (int i = 2; i * i <= n; i++)
    {
        if (n % i == 0)
            return false;
    }
    return true;
}

// Szállak által futtatot függvény
void *find_primes(void *arg)
{
    SharedData *data = (SharedData *)arg;
    int local_prime_count = 0;

    while (1)
    {
        int number_to_check;

        // zárolás, elvesszük a zárt amig teszteljük a következő elemet
        pthread_mutex_lock(&(data->mutex));

        number_to_check = data->current_number;
        data->current_number++; // léptetés a következő számra / tagra

        // feloldás, vissza tesszüka lakatot
        pthread_mutex_unlock(&(data->mutex));
        // Ha túllépjük a határt a szál befejezi a munkát, kilép a ciklusból
        if (number_to_check > data->max_nummber)
        {
            break;
        }
        // a nehezeb számmítást a lakaton kívül végezzük hogy tudjanak a szálak haladni
        if (is_prime(number_to_check))
        {
            local_prime_count++;
        }
    }

    // Amikor a szál végzett a maga részével, akkor hozzá adja a közös összrghez
    pthread_mutex_lock(&(data->mutex));
    data->total_primes += local_prime_count;
    pthread_mutex_unlock(&(data->mutex));
    pthread_exit(NULL);
}
int main()
{
    int maxNUM = 1000;
    pthread_t threads[THREADS];
    SharedData sharedata;

    sharedata.current_number = 0;
    sharedata.max_nummber = maxNUM;
    sharedata.total_primes = 0;

    pthread_mutex_init(&sharedata.mutex, NULL);

    printf("Kereses inditasa %d szallal........\n", THREADS);

    // 1. Szálak elindítása (Create the threads)
    for (int i = 0; i < THREADS; i++)
    {
        if (pthread_create(&threads[i], NULL, find_primes, &sharedata) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    // 2. Várakozás a szálak befejezésére (Wait for threads to finish)
    for (int i = 0; i < THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // mutex megsemmisítése
    pthread_mutex_destroy(&sharedata.mutex);
    
    printf("0-tol %d-ig %d darab primszam van.\n", maxNUM, sharedata.total_primes);

    return 0;
}