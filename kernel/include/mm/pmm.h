#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stdbool.h>

extern volatile struct limine_memmap_request memmap_request;
extern volatile struct limine_hhdm_request hhdm_request;

typedef struct BitmapEntry {
    struct BitmapEntry* next_entry;
    size_t available_pages;
    size_t capacity;
    uintptr_t base;
    uint64_t bitmap[];
} bitmap_entry_t;

static inline void* paddr_to_vaddr(void* paddr){
    return paddr + hhdm_request.response->offset;
}

/// @brief allocate pages PMM
/// @param page_count page count (bytes / 4096) will allocate to the nearist 4096 bytes tho
/// @return page base ptr
void *alloc_page(size_t page_count);

/// @brief frees the page(s)
/// @param paddr base of the start of the free.
/// @param page_count ammount of pages to free
void free_page(void* paddr, size_t page_count);

/// @brief gets the size of physical memory available
/// @return unsigned 64 memory size in bytes
size_t get_usable_pmem_size();

/// @brief gets the highest physical address
/// @return highest paddr
uintptr_t get_max_paddr();

/// @brief Physical Memory Management
/// @return success
uint8_t init_pmm();


#endif