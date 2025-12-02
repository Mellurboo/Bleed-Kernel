#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <mm/pmm.h>
#include <mm/paging.h>
#include <ansii.h>
#include <lib/limine/limine.h>

#define PAGE_SIZE           4096
#define PADDR_ENTRY_MASK    0x000FFFFFFFFFF000

extern volatile struct limine_memmap_request memmap_request;

static void zero_page_frame(void* paddr){
    void* v = paddr_to_vaddr(paddr);
    memset(v, 0, PAGE_SIZE);
}

static uint64_t alloc_empty_frame(void **vaddr){
    uint64_t paddr = (uint64_t)alloc_page(1);
    zero_page_frame((void *)paddr);
    if (vaddr) *vaddr = paddr_to_vaddr((void *)paddr);
    return paddr;
}

// walk the table limine gave us and populate it
uint64_t write_table_entry(uint64_t* table, size_t index, uint64_t flags){
    uint64_t entry = table[index];

    if (entry & PTE_PRESENT){
        return entry & PADDR_ENTRY_MASK;
    }

    void* new_entry_vaddr;
    uint64_t new_entry_paddr = alloc_empty_frame(&new_entry_vaddr);

    table[index] = new_entry_paddr | flags;

    return new_entry_paddr;
}

void walk_page_tables(uint64_t vaddr, uint64_t** out_page_table, size_t* out_page_table_index){
    void* pml4_vaddr = paddr_to_vaddr((void *)(read_cr3() & PADDR_ENTRY_MASK));

    size_t pml4_index   = (vaddr >> 39) & 0x1FF;
    size_t pdpt_index   = (vaddr >> 30) & 0x1FF;
    size_t pd_index     = (vaddr >> 21) & 0x1FF;
    size_t pt_index     = (vaddr >> 12) & 0x1FF;

    void* pdpt  = paddr_to_vaddr((void *)write_table_entry(pml4_vaddr, pml4_index, PTE_PRESENT | PTE_WRITABLE));
    void* pd    = paddr_to_vaddr((void *)write_table_entry(pdpt, pdpt_index, PTE_PRESENT | PTE_WRITABLE));
    void* pt    = paddr_to_vaddr((void *)write_table_entry(pd, pd_index, PTE_PRESENT | PTE_WRITABLE));

    *out_page_table = pt;
    *out_page_table_index = pt_index;
}

// install pages
void map_page(uint64_t paddr, uint64_t vaddr, uint64_t flags){
    uint64_t *pt;
    size_t pt_index;

    walk_page_tables(vaddr, &pt, &pt_index);
    pt[pt_index] = (paddr & PADDR_ENTRY_MASK) | flags;

    __asm__ volatile ("invlpg (%0)" :: "r"(vaddr) : "memory");
}

void extend_paging(){
    struct limine_memmap_response* mmap = memmap_request.response;
    for (size_t i = 0; i < mmap->entry_count; i++){
        struct limine_memmap_entry *entry = mmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE){   // this operation is very slow
            for (uint64_t paddr = entry->base; paddr < entry->base + entry->length; paddr += PAGE_SIZE){
                uint64_t vaddr = (uint64_t)paddr_to_vaddr((void *)paddr);
                map_page(paddr, vaddr, PTE_PRESENT | PTE_WRITABLE);
            }
        }
    }

    kprintf(LOG_OK "Paging Extended to all Available Memory\n");
}
