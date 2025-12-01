#include <stdint.h>
#include <stdio.h>
#include <ansii.h>
#include <lib/limine/limine.h>
#include <x86_64/gdt/gdt.h>
#include <x86_64/idt/idt.h>
#include <string.h>
#include <panic.h>
#include <mm/pmm.h>
#include <mm/addr.h>

void kmain(){
    init_gdt();
    init_idt();
    kprintf(LOG_INFO "Physical Memory: %lluMiB\n", get_usable_pmem_size() / 1024 / 1024);
    kprintf(LOG_INFO "Highest Free PADDR: 0x%p\n", get_max_paddr());
    init_pmm();

    
    for (;;) {}
    kpanic("KERNEL_FINISHED_EXECUTION");
}