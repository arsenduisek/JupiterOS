#ifndef JUPITER_HEAP_H
#define JUPITER_HEAP_H

#include "types.h"

void  heap_init(void);
void *kmalloc(size_t size);
void  kfree(void *ptr);
size_t heap_used(void);

#endif
