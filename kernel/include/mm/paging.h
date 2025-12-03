#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PTE_PRESENT     (1ULL<<0)
#define PTE_WRITABLE    (1ULL<<1)
#define PTE_PS          (1ULL<<7)


static inline uint64_t read_cr3(){
    uint64_t cr3;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3) :: "memory");
    return cr3;
}

static inline void write_cr3(uint64_t cr3){
    __asm__ volatile ("mov %0, %%cr3" :: "r"(cr3) : "memory");
}

void extend_paging();

#endif