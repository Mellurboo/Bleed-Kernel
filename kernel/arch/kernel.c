#include <stdint.h>
#include <stdio.h>
#include <ascii.h>
#include <lib/limine/limine.h>
#include <x86_64/gdt/gdt.h>
#include <x86_64/idt/idt.h>
#include <mm/pmm.h>

void kmain(){
    init_gdt();
    init_idt();
    kprintf(LOG_OK "Total Available Memory: %llu MiB\n", get_usable_pmem_size() / 1024 / 1024);
    for (;;) {}
}