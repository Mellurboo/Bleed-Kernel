#ifndef ADDR_H
#define ADDR_H

#include <stdint.h>

uintptr_t paddr_to_vaddr(uintptr_t paddr);
uintptr_t vaddr_to_paddr(uintptr_t vaddr);

#endif