#include <stdint.h>
#include <stdio.h>
#include <ansii.h>
#include <lib/limine/limine.h>
#include <x86_64/gdt/gdt.h>
#include <x86_64/idt/idt.h>
#include <string.h>
#include <panic.h>
#include <mm/pmm.h>

void kmain(){
    init_gdt();
    init_idt();
    kprintf(LOG_INFO "Physical Memory: %lluMiB\n", get_usable_pmem_size() / 1024 / 1024);
    kprintf(LOG_INFO "Highest Free PADDR: 0x%p\n", get_max_paddr());
    init_pmm();

    void* paddr = alloc_page(1);
    uint64_t* vaddr = (uint64_t*)paddr_to_vaddr(paddr);
    *vaddr = 6767;

    kprintf(LOG_OK "Stored a number:\n\tpaddr: 0x%p\n\tvaddr: @ 0x%p (with a value of %llu)\n", paddr, vaddr, *vaddr);
    free_page(paddr, 1);
    
    for (;;) {}
    kpanic("KERNEL_FINISHED_EXECUTION");
}