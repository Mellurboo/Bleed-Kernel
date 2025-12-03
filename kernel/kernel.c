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

    for (;;) {}
    kpanic("KERNEL_FINISHED_EXECUTION");
}