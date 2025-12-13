#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <mm/pmm.h>
#include <mm/paging.h>
#include <ansii.h>
#include <vendor/limine/limine.h>
#include <drivers/serial/serial.h>

#define PAGE_SIZE_4K       4096
#define PAGE_SIZE_2M       (512 * PAGE_SIZE_4K)
#define PADDR_ENTRY_MASK   0x000FFFFFFFFFF000ULL

extern volatile struct limine_memmap_request memmap_request;

static uint64_t alloc_empty_frame(void **vaddr) {
    paddr_t paddr = alloc_pages(1);
    if(!paddr) {}
        //kprintf(LOG_ERROR "Page Allocation Failed\n");

    void* v = paddr_to_vaddr(paddr);
    if (v) memset(v, 0, PAGE_SIZE_4K);

    if (vaddr) *vaddr = v;
    return paddr;
}

uint64_t write_table_entry(uint64_t* table, size_t index, uint64_t flags) {
    uint64_t entry = table[index];
    if (entry & PTE_PRESENT) return entry & PADDR_ENTRY_MASK;

    void* new_entry_vaddr = NULL;
    uint64_t new_entry_paddr = alloc_empty_frame(&new_entry_vaddr);
    if (!new_entry_vaddr) return 0;

    table[index] = (new_entry_paddr & PADDR_ENTRY_MASK) | (flags & ~PADDR_ENTRY_MASK);
    return new_entry_paddr & PADDR_ENTRY_MASK;
}

void walk_page_tables(uint64_t vaddr, uint64_t **out_pd, size_t *out_pd_index) {
    uint64_t cr3 = read_cr3() & PADDR_ENTRY_MASK;
    uint64_t* pml4_vaddr = (uint64_t*)paddr_to_vaddr(cr3);

    size_t pml4_index = (vaddr >> 39) & 0x1FF;
    size_t pdpt_index = (vaddr >> 30) & 0x1FF;
    size_t pd_index   = (vaddr >> 21) & 0x1FF;

    uint64_t p_pdpt = write_table_entry(pml4_vaddr, pml4_index, PTE_PRESENT | PTE_WRITABLE);
    uint64_t* pdpt = (uint64_t*)paddr_to_vaddr(p_pdpt);

    uint64_t p_pd = write_table_entry(pdpt, pdpt_index, PTE_PRESENT | PTE_WRITABLE);
    uint64_t* pd = (uint64_t*)paddr_to_vaddr(p_pd);

    *out_pd = pd;
    *out_pd_index = pd_index;
}

void map_page(uint64_t paddr, uint64_t vaddr, uint64_t flags) {
    uint64_t *pd;
    size_t pd_index;

    walk_page_tables(vaddr, &pd, &pd_index);
    if (!pd) return;

    pd[pd_index] = (paddr & PADDR_ENTRY_MASK) | (flags & ~PADDR_ENTRY_MASK) | PTE_PRESENT | PTE_WRITABLE | PTE_PS;
    __asm__ volatile ("invlpg (%0)" :: "r"(vaddr) : "memory");
}

void extend_paging() {
    struct limine_memmap_response* mmap = memmap_request.response;

    //kprintf(LOG_INFO "Paging: memmap entry count = %llu\n", mmap->entry_count);

    for (size_t i = 0; i < mmap->entry_count; i++) {
        struct limine_memmap_entry* entry = mmap->entries[i];
        if (entry->type != LIMINE_MEMMAP_USABLE) continue;

        uint64_t mapped = 0;
        uint64_t base = entry->base & ~(PAGE_SIZE_2M - 1);
        uint64_t end = entry->base + entry->length;

        for (uint64_t paddr = base; paddr + PAGE_SIZE_2M <= end; paddr += PAGE_SIZE_2M) {
            void* hv = paddr_to_vaddr(paddr);
            if (!hv) continue;

            map_page(paddr, (paddr_t)hv, 0);
            mapped++;
        }
    }

    //kprintf(LOG_OK "Page Tables Reloaded\n");
}
