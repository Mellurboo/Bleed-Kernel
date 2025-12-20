#include <vendor/limine_bootloader/limine.h>
#include <drivers/serial/serial.h>
#include <mm/pmm.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ansii.h>
#include <stddef.h>
#include <string.h>
#include <mm/pmm.h>
#include <status.h>

#define FRAME_USED      1
#define FRAME_FREE      0

#define PAGE_ALIGN_DOWN(x)  (((x) / PAGE_SIZE)*PAGE_SIZE)
#define PAGE_ALIGN_UP(x)    ((((x) + (PAGE_SIZE-1))/ PAGE_SIZE)*PAGE_SIZE)

static bitmap_entry_t* bitmap_head = NULL;

/// @brief get the highest physical address
/// @return uintptr to it
uintptr_t get_max_paddr(){
    struct limine_memmap_response *mmap = memmap_request.response;

    uint64_t max = 0;
    for (uint64_t i = 0; i < mmap->entry_count; i++){
        struct limine_memmap_entry *entry = mmap->entries[i];

        if (entry->type != LIMINE_MEMMAP_USABLE) continue;
        uint64_t end = entry->base + entry->length;

        if (end > max) max = end;
    }

    return (uintptr_t)max;
}

/// @brief gets the size of physical memory available
/// @return unsigned 64 memory size in bytes
size_t paging_get_usable_mem_size(){
    struct limine_memmap_response* mmap = memmap_request.response;

    size_t bytes = 0;
    for (size_t i = 0; i < mmap->entry_count; i++){
        struct limine_memmap_entry *entry = mmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE){
            bytes += entry->length;
        }
    }
    return bytes;
}

/// @brief make a bitmap entry as unavailable
/// @param entry bitmap entry target
/// @param start indexed location
/// @param page_count pages to make unavailable
void paging_mark_entry_unavailable(bitmap_entry_t* entry, size_t start, size_t page_count){
    for (size_t i = 0; i < page_count; i++){
        size_t index = start + i;
        entry->bitmap[index / 8] |= (FRAME_USED << (index % 8));
    }
    entry->available_pages -= page_count;
}

/// @brief make a bitmap entry as available
/// @param entry bitmap entry target
/// @param start indexed location
/// @param page_count pages to make unavailable
static void paging_mark_entry_available(bitmap_entry_t* entry, size_t start, size_t page_count){
    for (size_t i = 0; i < page_count; i++){
        size_t index = start + i;
        entry->bitmap[index / 8] &= ~(FRAME_USED << (index % 8));
    }
    entry->available_pages += page_count;
}

/// @brief Physical Memory Management
/// @return success
uint8_t pmm_init() {
    struct limine_memmap_response* mmap = memmap_request.response;
    struct limine_hhdm_response* hhdm = hhdm_request.response;
    bitmap_entry_t** prev_tail = &bitmap_head;
    for (uint64_t i = 0; i < mmap->entry_count; i++) {
        if (mmap->entries[i]->type != LIMINE_MEMMAP_USABLE)
            continue;

        bitmap_entry_t* bmentry = (bitmap_entry_t*)(mmap->entries[i]->base + hhdm->offset);

        *prev_tail = bmentry;
        bmentry->next_entry = NULL;
        prev_tail = &bmentry->next_entry;

        bmentry->capacity = mmap->entries[i]->length / PAGE_SIZE;
        bmentry->available_pages = bmentry->capacity;
        
        memset(bmentry->bitmap, FRAME_FREE, ((bmentry->capacity) + 7) / 8);
        size_t bitmap_header_page_size = PAGE_ALIGN_UP(sizeof(bitmap_entry_t) + (bmentry->capacity + 7) / 8) / PAGE_SIZE;
        paging_mark_entry_unavailable(bmentry, 0, bitmap_header_page_size);

        serial_printf(LOG_INFO "Usable Space Bitmap created: %p of %d pages\n", (void *)bmentry, bmentry->available_pages);
    }
    
    serial_printf(LOG_OK "PMM Initialised\n");
    return 0;
}

/// @brief Finds a contiguous block of free bits in a bitmap.
/// @param entry bitmap entry structure
/// @param count The number of consecutive free bits required
/// @return The starting index of the first contiguous free block if found, or -1 if no such block exists.
static int64_t paging_bitmap_find_free(bitmap_entry_t* entry, size_t count){
    size_t free_extention = 0;
    size_t start = 0;

    for (size_t i = 0; i < entry->capacity; i++){
        size_t byte = entry->bitmap[i / 8];
        if (!(byte & (1 << (i % 8)))){
            if (free_extention == 0) start = i;
            free_extention++;
            if (free_extention == count) return start;
        }else{
            free_extention = 0;
        }
    }

    return -1;
}

/// @brief allocate pages PMM
/// @param page_count page count (bytes / 4096) will allocate to the nearist 4096 bytes tho
/// @return page base ptr
paddr_t paging_alloc_pages(size_t page_count){
    if (page_count == 0) return 0;
    struct limine_hhdm_response* hhdm = hhdm_request.response;
    for (bitmap_entry_t* head = bitmap_head; head != NULL; head = head->next_entry){
        if (head->available_pages >= page_count){
            int64_t start = paging_bitmap_find_free(head, page_count);

            if (start < 0)
                continue;

            paging_mark_entry_unavailable(head, start, page_count);
            uintptr_t paddr = (((paddr_t)head) - hhdm->offset) + (start * PAGE_SIZE);

            return paddr;
        }
    }
    return -OUT_OF_MEMORY;
}

/// @brief frees the page(s)
/// @param paddr base of the start of the free.
/// @param page_count ammount of pages to free
void paging_free_pages(paddr_t paddr, size_t page_count){
    for (bitmap_entry_t* entry = bitmap_head; entry != NULL; entry = entry->next_entry){
        uintptr_t entry_start = ((paddr_t)entry) - hhdm_request.response->offset;
        uintptr_t entry_end = entry_start + entry->capacity * PAGE_SIZE;

        if ((uintptr_t)paddr >= entry_start && (uintptr_t)paddr < entry_end){
            size_t start = ((uintptr_t)paddr - entry_start) / PAGE_SIZE;
            paging_mark_entry_available(entry, start, page_count);
            return;
        }
    }
}