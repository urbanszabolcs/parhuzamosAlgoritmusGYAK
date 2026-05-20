#include <windows.h>
#include <stdio.h>

int main(){
    LARGE_INTEGER frequency;
    LARGE_INTEGER start, end;
    double time_spent;

    //Órajel-frekvencia lekérése
    QueryPerformanceCounter(&frequency);

    //Mérés indítása
    QueryPerformanceCounter(&start);

    ///Futtatandó kód

    //Mérés leállítása
    QueryPerformanceCounter(&end);

    //Eltelt idő kiszámítása
    time_spent = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;

    printf("Futasi ido: %f masodpercben\n", time_spent);
    return 0;
}