#include <stdio.h>
#include <unistd.h>
#include "memarena.h"

extern header_t *head;

void print_heap() {
    header_t *curr = head;
    printf("\n--- Heap State ---\n");
    if (!curr) {
        printf("Empty Heap\n");
    }
    
    int counter = 0;
    while(curr) {
        printf("Block %d: [%p] Size: %zu bytes | Free: %s | Next: %p\n", 
               counter++, 
               (void*)curr, 
               curr->size, 
               curr->is_free ? "YES" : "NO", 
               (void*)curr->next);
        curr = curr->next;
    }
    printf("------------------\n\n");
}

int main() {
    printf("=== TEST 1: Basic Allocation ===\n");
    int *arr = (int*)my_malloc(10 * sizeof(int));
    
    if (arr) {
        for(int i = 0; i < 10; i++) arr[i] = i * 10;
        printf("Allocated arr at %p\n", (void*)arr);
    }
    print_heap();


    printf("=== TEST 2: Splitting ===\n");
    my_free(arr);
    printf("Freed 'arr'. Heap should have 1 free block.\n");
    print_heap();

    char *small_chunk = (char*)my_malloc(8); 
    printf("Requested 8 bytes. Heap should have [Used: 8] -> [Free: Remainder]\n");
    print_heap();


    printf("\n=== TEST 3: Coalescing (The Carving Strategy) ===\n");
    
    int *big_block = (int*)my_malloc(128);
    printf("Allocated Big Block at %p\n", (void*)big_block);
    
    my_free(big_block);
    printf("Freed Big Block. We now have a large hole to fill.\n");

    void *a = my_malloc(16);
    void *b = my_malloc(16);
    void *c = my_malloc(16);
    
    printf("Allocated A, B, C from the hole.\n");
    print_heap();

    printf("Freeing B (Middle)...\n");
    my_free(b);
    print_heap();

    printf("Freeing C (Right)... Should merge B+C\n");
    my_free(c);
    print_heap();

    printf("Freeing A (Left)... Should merge A+(B+C)\n");
    my_free(a);
    print_heap();

    return 0;
}