#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <mm/pmm.h>

#define PTE_PRESENT     (1ULL<<0)
#define PTE_WRITABLE    (1ULL<<1)
#define PTE_USER        (1ULL<<2)
#define PTE_PS          (1ULL<<7)

#define PAGE_KERNEL_RW      (PTE_WRITABLE | PTE_PS)
#define PAGE_KERNEL_RO      (PTE_PS)
#define PAGE_USER_RW        (PTE_WRITABLE | PTE_USER)
#define PAGE_USER_RO        (PTE_USER)


extern paddr_t cr3_paddr;
extern paddr_t kernel_page_map;

static inline uint64_t read_cr3(){
    uint64_t cr3;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3) :: "memory");
    return cr3;
}

static inline void write_cr3(uint64_t cr3){
    __asm__ volatile ("mov %0, %%cr3" :: "r"(cr3) : "memory");
}

void paging_map_page(uint64_t paddr, uint64_t vaddr, uint64_t flags);
paddr_t paging_create_address_space(void);
void paging_switch_address_space(paddr_t cr3);
void paging_destroy_address_space(paddr_t cr3);

void reinit_paging();

#endif