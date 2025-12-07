#include <mm/pmm.h>
#include <stddef.h>

void* kmalloc(size_t bytes){
    paddr_t p = alloc_pages((bytes + (PAGE_SIZE - 1)) / PAGE_SIZE);
    if (p == 0){
        return NULL;
    }

    return paddr_to_vaddr(p);
}

void kfree(void* addr, size_t bytes){
    free_pages(vaddr_to_paddr(addr), (bytes + (PAGE_SIZE - 1)) / PAGE_SIZE);
}