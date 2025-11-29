#include <lib/limine/limine.h>
#include <stdint.h>

extern volatile struct limine_memmap_request memmap_request;

/// @brief gets the size of physical memory available
/// @return unsigned 64 memory size in bytes
uint64_t get_usable_pmem_size(){
    struct limine_memmap_response *mmap = memmap_request.response;
    if (!mmap) return 0;

    uint64_t bytes = 0;
    for (uint64_t i = 0; i < mmap->entry_count; i++){
        struct limine_memmap_entry *entry = mmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE){
            bytes += entry->length;
        }
    }

    return bytes;
}