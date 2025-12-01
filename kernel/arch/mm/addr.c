#include <stdint.h>
#include <lib/limine/limine.h>

struct limine_hhdm_response response;

uintptr_t paddr_to_vaddr(uintptr_t paddr){
    return paddr + response.offset;
}

uintptr_t vaddr_to_paddr(uintptr_t vaddr){
    return vaddr - response.offset;
}