#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(){
    srand(time(NULL));
    
    // 0 és 99 közötti random szám generálása
    int r1 = rand() % 100;

    printf("%d", r1);
    return 0;
}