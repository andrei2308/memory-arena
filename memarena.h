#ifndef MEMARENA_H
#define MEMARENA_H

#include <stddef.h>
#include <pthread.h>

// move the struct definition HERE so main.c can see it
typedef struct header_t {
    size_t size;
    unsigned is_free;
    struct header_t *next;
    struct header_t *prev;
} header_t;

extern header_t *head;

void *my_malloc(size_t size);
void my_free(void *block);

#endif