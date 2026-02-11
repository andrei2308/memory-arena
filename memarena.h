#ifndef MEMARENA_H
#define MEMARENA_H

#include <stddef.h>

void *my_malloc(size_t size);
void my_free(void *block);

#endif