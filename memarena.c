#include <unistd.h>
#include <stddef.h>
#include <pthread.h>
#include <sys/mman.h>

#define ALIGNMENT 8 // we can change this to whatever but it must be a power of 2
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1)) // size round up 

typedef struct header_t {
    size_t size;        // size of the block not including the header
    unsigned is_free;  
    struct header_t *next;  
} header_t;

// linked list of memory blocks
header_t *head = NULL; 
header_t *tail = NULL; 
pthread_mutex_t global_malloc_lock; 

// find a free block in the list
header_t *get_free_block(size_t size) {
    header_t *curr = head;
    while(curr) {
        if (curr->is_free && curr->size >= size)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

// split block 
void split_block(header_t *block, size_t requested_size){

    size_t min_block_size = sizeof(header_t);

    if(block->size > requested_size + min_block_size){

        // move the pointer in the current block after the memory that will be given to the user
        header_t* remainder = (header_t*)((char*)block + min_block_size + requested_size);

        // calculate the remaining size after giving memory to the user
        // mark block as free
        remainder->size = block->size - min_block_size - requested_size;
        remainder->is_free = 1;
        remainder->next = block->next;

        // change block size to aligned size
        block->size = requested_size;
        block->next = remainder;
    }
}

void *my_malloc(size_t size) {
    size_t total_size;
    void *block;
    header_t *header;

    if (size == 0)
        return NULL;

    pthread_mutex_lock(&global_malloc_lock);

    size = ALIGN(size);

    header = get_free_block(size);
    if (header) {
        split_block(header,size);

        header->is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        return (void*)(header + 1); // return a pointer to user data, 8 bytes after the header
    }

    total_size = sizeof(header_t) + size;

    // request memory from the OS if no suitable block is found
    block = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (block == MAP_FAILED) {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL; 
    }

    // add the new block to the linked list
    header = block;
    header->size = size;
    header->is_free = 0;
    header->next = NULL;

    if (!head) 
        head = header;
    if (tail) 
        tail->next = header;
    
    tail = header;
    
    pthread_mutex_unlock(&global_malloc_lock);
    return (void*)(header + 1); // return pointer to user data, 8 bytes after the header
}

void my_free(void *block) {
    header_t *header;

    if (!block)
        return;

    pthread_mutex_lock(&global_malloc_lock);

    header = (header_t*)block - 1;

    // for now just mark it as free
    // we will later implement the bins and merging of free blocks as also splitting and returning memory to the OS
    header->is_free = 1;

    pthread_mutex_unlock(&global_malloc_lock);
}