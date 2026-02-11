#include <stdio.h>
#include "memarena.h"

int main() {
    printf("Allocating memory...\n");
    int *arr = (int*)my_malloc(10 * sizeof(int));
    
    if (arr) {
        printf("Memory allocated successfully!\n");
        for(int i = 0; i < 10; i++) {
            arr[i] = i * 10;
            printf("%d ", arr[i]);
        }
        printf("\n");
        
        printf("Freeing memory...\n");
        my_free(arr);
        printf("Memory freed!\n");
    }
    
    return 0;
}