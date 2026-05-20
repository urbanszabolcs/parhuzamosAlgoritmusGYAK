#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(){
    clock_t start = clock();

    // futtatandó kód

    clock_t end = clock();

    //futási idő mp-ben
    double time_spent = (double)(end - start);

    printf("Futasi ido: %f masodperc\n",time_spent);

    return 0;
}