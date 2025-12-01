#include <lib/limine/limine.h>
#include <mm/pmm.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ansii.h>
#include <stddef.h>
#include <string.h>
#include <panic.h>

#define PAGE_SIZE       4096
#define FRAME_USED      1
#define FRAME_FREE      0

#define PAGE_ALIGN_DOWN(x) (((x) / PAGE_SIZE)*PAGE_SIZE)
#define PAGE_ALIGN_UP(x) ((((x) + (PAGE_SIZE-1))/ PAGE_SIZE)*PAGE_SIZE)

extern volatile struct limine_memmap_request memmap_request;
extern volatile struct limine_hhdm_request hhdm_request;

static bitmap_entry_t* bitmap_head = NULL;

uintptr_t get_max_paddr(){
    struct limine_memmap_response *mmap = memmap_request.response;
    if (!mmap) kpanic("MEMORY_MAP_NOT_AVAILABLE");

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
size_t get_usable_pmem_size(){
    struct limine_memmap_response* mmap = memmap_request.response;
    if (!mmap) kpanic("MEMORY_MAP_NOT_AVAILABLE");

    size_t bytes = 0;
    for (size_t i = 0; i < mmap->entry_count; i++){
        struct limine_memmap_entry *entry = mmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE){
            bytes += entry->length;
        }
    }
    return bytes;
}

void entry_mark_unavailable(bitmap_entry_t* entry, size_t start, size_t page_count){
    for (size_t i = 0; i < page_count; i++){
        size_t index = start + i;
        entry->bitmap[index / 8] |= (FRAME_USED << (index % 8));
    }
    entry->available_pages -= page_count;
}

void entry_mark_available(bitmap_entry_t* entry, size_t start, size_t page_count){
    for (size_t i = 0; i < page_count; i++){
        size_t index = start + i;
        entry->bitmap[index / 8] &= ~(FRAME_USED << (index % 8));
    }
    entry->available_pages += page_count;
}

/// @brief Physical Memory Management
/// @return success
uint8_t init_pmm() {
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
        bmentry->base = mmap->entries[i]->base;

        size_t bitmap_header_page_size = PAGE_ALIGN_UP(sizeof(bitmap_entry_t) + (bmentry->capacity + 7) / 8) / PAGE_SIZE;
        entry_mark_unavailable(bmentry, 0, bitmap_header_page_size);
        
        memset(bmentry->bitmap, FRAME_FREE, ((bmentry->capacity) + 7) / 8);
        kprintf(LOG_OK "Usable Space Bitmap created: 0x%p | %llu Pages (%llu KiB) | Available Pages: %llu\n", (void *)bmentry, bmentry->capacity, ((bmentry->capacity)*PAGE_SIZE)/1024, bmentry->available_pages);
    }
    kprintf(LOG_OK "PMM Initialised\n");
    return 0;
}

static int64_t bitmap_find_free(bitmap_entry_t* entry, size_t count){
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
void *alloc_page(size_t page_count){
    struct limine_hhdm_response* hhdm = hhdm_request.response;
    for (bitmap_entry_t* head = bitmap_head; head != NULL; head = head->next_entry){
        if (head->available_pages >= page_count){
            int64_t start = bitmap_find_free(head, page_count);
            if (start < 0) continue;

            entry_mark_unavailable(head, start, page_count);
            uintptr_t paddr = head->base + (start * PAGE_SIZE);
            return (void *)(paddr + hhdm_request.response->offset);
        }else continue;
    }
    return NULL;
}

/// @brief frees the page(s)
/// @param paddr base of the start of the free.
/// @param page_count ammount of pages to free
void free_page(void* paddr, size_t page_count){
    for (bitmap_entry_t* entry = bitmap_head; entry != NULL; entry = entry->next_entry){
        uintptr_t entry_start = (uintptr_t)entry->base;
        uintptr_t entry_end = entry_start + entry->capacity * PAGE_SIZE;

        if ((uintptr_t)paddr >= entry_start && (uintptr_t)paddr < entry_end){
            size_t start = ((uintptr_t)paddr - entry_start) / PAGE_SIZE;
            entry_mark_available(entry, start, page_count);
            return;
        }
    }
}