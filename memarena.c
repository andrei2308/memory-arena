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
    struct header_t *prev;
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
        remainder->prev = block;

        if(remainder->next){
            remainder->next->prev = remainder;
        }

        // change block size to aligned size
        block->size = requested_size;
        block->next = remainder;

        if(block == tail){
            tail = remainder;
        }
    }
}

// merging function
// at the moment, it only does front merging
void coalesce(header_t *curr) {
    
   if (curr->next && curr->next->is_free) {
        void *curr_end = (char*)curr + sizeof(header_t) + curr->size;
        if (curr_end == (void*)curr->next) {
            
            curr->size += sizeof(header_t) + curr->next->size;
            
            curr->next = curr->next->next;
            if (curr->next) {
                curr->next->prev = curr;
            } else {
                tail = curr;
            }
        }
    }

    if(curr->prev && curr->prev->is_free){
        
        void *prev_end = (char*)curr->prev + sizeof(header_t) + curr->prev->size;

        if(prev_end == (void*)curr){
            curr->prev->size += sizeof(header_t) + curr->size;

            curr->prev->next = curr->next;

            if(curr->next){
                curr->next->prev = curr->prev;
            } else{
                tail = curr->prev;
            }
        }
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
    header->prev = tail;

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
    // we will later implement the bins
    header->is_free = 1;

    coalesce(header);

    pthread_mutex_unlock(&global_malloc_lock);
}