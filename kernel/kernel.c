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

#define TTY_BACKGROUND  0, 10, 41

void kmain(){
    fb_fill(TTY_BACKGROUND);
    set_tty_bg_colour(TTY_BACKGROUND);
    set_tty_fg_colour(255, 255, 255);

    init_gdt();
    init_idt();
    kprintf(LOG_INFO "Physical Memory: %lluMiB\n", get_usable_pmem_size() / 1024 / 1024);
    kprintf(LOG_INFO "Highest Free PADDR: 0x%p\n", get_max_paddr());
    init_pmm();
    extend_paging();

    inode_t* root = tempfs_create_directory(NULL, "/");
    inode_t* file = tempfs_create_file(root, "test.txt");

    const char* file_content = "a file, in my kernel? it's not as unlikely as you'd think\0";
    size_t w = tempfs_write(file, file_content, strlen(file_content), 0);

    char buffer[64];
    size_t bytes = tempfs_read(file, buffer, sizeof(buffer), 0);
    kprintf("%s\n", buffer);

    for (;;) {}
    kpanic("KERNEL_FINISHED_EXECUTION");
}