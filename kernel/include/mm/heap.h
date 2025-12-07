#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>

void* kmalloc(size_t bytes);
void kfree(void* addr, size_t bytes);

#endif