#include <stdint.h>
#include <stdio.h>
#include <ansii.h>
#include <lib/limine/limine.h>
#include <x86_64/gdt/gdt.h>
#include <x86_64/idt/idt.h>h>
#include <mm/pmm.h>

void kmain(){
    init_gdt();
    init_idt();
    kprintf("Physical Memory: %lluMiB\n", get_usable_pmem_size() / 1024 / 1024);
    kprintf("Highest Free PADDR: 0x%p\n", get_max_paddr());
    init_pmm();
    
    for (;;) {}
}