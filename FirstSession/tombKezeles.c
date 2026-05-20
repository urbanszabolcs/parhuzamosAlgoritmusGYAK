#include <stdio.h>
#include <stdlib.h>

int main(){
    int rows = 1000;
    int cols = 1000;

    //Folytonos memóriafoglalás a mátrixnak
    int *matrix = (int*)malloc(rows * cols * sizeof(int));

    //indexelés: sor * oszlopszám + oszlop
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; i < cols; i++)
        {
            matrix[i * cols + j] = 0; //Hozzáférés a (i, j ) elemhez
        }
        
    }
    

    free(matrix);
    return 0;
}
