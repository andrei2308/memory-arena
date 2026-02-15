#ifndef MEMARENA_H
#define MEMARENA_H

#include <stddef.h>
#include <pthread.h> // Needed for mutex in the struct if you were storing it there, 
                     // but mostly for consistency if you expand later.

// move the struct definition HERE so main.c can see it
typedef struct header_t {
    size_t size;
    unsigned is_free;
    struct header_t *next;
} header_t;

// 2. Expose the head pointer so main.c can iterate the list
extern header_t *head;

// 3. Function Prototypes
void *my_malloc(size_t size);
void my_free(void *block);

#endif