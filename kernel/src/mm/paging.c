#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <mm/pmm.h>
#include <mm/paging.h>
#include <ansii.h>
#include <vendor/limine/limine.h>
#include <drivers/serial/serial.h>
#include <panic.h>
#include <sched/scheduler.h>

#define PAGE_SIZE_4K       4096
#define PAGE_SIZE_2M       (512 * PAGE_SIZE_4K)
#define PADDR_ENTRY_MASK   0x000FFFFFFFFFF000ULL

paddr_t kernel_page_map = 0;

extern volatile struct limine_memmap_request memmap_request;

static uint64_t paging_alloc_empty_frame(void **vaddr) {
    paddr_t paddr = alloc_pages(1);
    if(!paddr) {}
        kprintf(LOG_ERROR "Page Allocation Failed, Out of Memory?\n");

    void* v = paddr_to_vaddr(paddr);
    if (v) memset(v, 0, PAGE_SIZE_4K);

    if (vaddr) *vaddr = v;
    return paddr;
}

uint64_t paging_write_table_entry(uint64_t* table, size_t index, uint64_t flags) {
    uint64_t entry = table[index];
    if (entry & PTE_PRESENT) return entry & PADDR_ENTRY_MASK;

    void* new_entry_vaddr = NULL;
    uint64_t new_entry_paddr = paging_alloc_empty_frame(&new_entry_vaddr);
    if (!new_entry_vaddr) return 0;

    table[index] = (new_entry_paddr & PADDR_ENTRY_MASK) | (flags & ~PADDR_ENTRY_MASK);
    return new_entry_paddr & PADDR_ENTRY_MASK;
}

void paging_walk_page_tables(uint64_t vaddr, uint64_t **out_pd, size_t *out_pd_index) {
    uint64_t cr3 = read_cr3() & PADDR_ENTRY_MASK;
    uint64_t* pml4_vaddr = (uint64_t*)paddr_to_vaddr(cr3);

    size_t pml4_index = (vaddr >> 39) & 0x1FF;
    size_t pdpt_index = (vaddr >> 30) & 0x1FF;
    size_t pd_index   = (vaddr >> 21) & 0x1FF;

    uint64_t p_pdpt = paging_write_table_entry(pml4_vaddr, pml4_index, PTE_PRESENT | PTE_WRITABLE);
    uint64_t* pdpt = (uint64_t*)paddr_to_vaddr(p_pdpt);

    uint64_t p_pd = paging_write_table_entry(pdpt, pdpt_index, PTE_PRESENT | PTE_WRITABLE);
    uint64_t* pd = (uint64_t*)paddr_to_vaddr(p_pd);

    *out_pd = pd;
    *out_pd_index = pd_index;
}

void paging_map_page(uint64_t paddr, uint64_t vaddr, uint64_t flags) {
    uint64_t *pd;
    size_t pd_index;

    paging_walk_page_tables(vaddr, &pd, &pd_index);
    if (!pd) return;

    pd[pd_index] = (paddr & PADDR_ENTRY_MASK) | (flags & ~PADDR_ENTRY_MASK) | PTE_PRESENT;
    __asm__ volatile ("invlpg (%0)" :: "r"(vaddr) : "memory");
}

void paging_init_kernel_map(void) {
    kernel_page_map = read_cr3() & PADDR_ENTRY_MASK;

    void *pml4_vaddr = paddr_to_vaddr(kernel_page_map);
    if (!pml4_vaddr) {
        ke_panic("Kernel PML4 not mapped");
    }

    serial_printf("Kernel Page Map = %p\n",
                  (void*)kernel_page_map);
}

void reinit_paging() {
    struct limine_memmap_response* mmap = memmap_request.response;

    for (size_t i = 0; i < mmap->entry_count; i++) {
        struct limine_memmap_entry* entry = mmap->entries[i];
        if (entry->type != LIMINE_MEMMAP_USABLE) continue;

        uint64_t mapped = 0;
        uint64_t base = entry->base & ~(PAGE_SIZE_2M - 1);
        uint64_t end = entry->base + entry->length;

        for (uint64_t paddr = base; paddr + PAGE_SIZE_2M <= end; paddr += PAGE_SIZE_2M) {
            void* hv = paddr_to_vaddr(paddr);
            if (!hv) continue;

            paging_map_page(paddr, (paddr_t)hv, PAGE_KERNEL_RW);
            mapped++;
        }
    }

    paging_init_kernel_map();
}

paddr_t paging_create_address_space(void){
    void* vaddr = NULL;
    paddr_t pml4_paddr = paging_alloc_empty_frame(&vaddr);

    if (!pml4_paddr) {
        serial_printf("Failed to allocate PML4\n"); 
        return 0;
    }
    
    uint64_t *kernel_pml4 = (uint64_t *)paddr_to_vaddr(kernel_page_map);
    uint64_t *new_pml4 = (uint64_t *)vaddr;

    memset(new_pml4, 0, PAGE_SIZE);

    for (size_t i = 256; i < 512; i++){
        new_pml4[i] = kernel_pml4[i] & ~PTE_USER;
    }

    serial_printf("Created address space at %p\n", (void*)pml4_paddr);
    return pml4_paddr;
}

void paging_switch_address_space(paddr_t cr3){
    asm volatile("mov %0, %%cr3" :: "r"(cr3) : "memory");
}

void paging_destroy_address_space(paddr_t cr3){
    if (!cr3) return;
    free_pages(cr3, 1);
}