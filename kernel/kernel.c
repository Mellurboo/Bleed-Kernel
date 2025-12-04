#include <stdint.h>
#include <stdio.h>
#include <ansii.h>
#include <lib/limine/limine.h>
#include <gdt/gdt.h>
#include <idt/idt.h>
#include <string.h>
#include <panic.h>
#include <mm/pmm.h>
#include <mm/paging.h>
#include <drivers/serial.h>
#include <drivers/framebuffer/framebuffer.h>
#include <fs/tempfs.h>

void kmain(){
    init_serial();
    serial_write("Bleed Serial Output:\n");

    init_gdt();
    init_idt();
    kprintf(LOG_INFO "Physical Memory: %lluMiB\n", get_usable_pmem_size() / 1024 / 1024);
    kprintf(LOG_INFO "Highest Free PADDR: 0x%p\n", get_max_paddr());
    init_pmm();
    extend_paging();

    inode_t* root = tempfs_create_directory(NULL, "/");
    inode_t* initrd = init_initrd(root);

    tempfs_list(root);
    tempfs_list(initrd);

    for (;;) {}
    kpanic("KERNEL_FINISHED_EXECUTION");
}