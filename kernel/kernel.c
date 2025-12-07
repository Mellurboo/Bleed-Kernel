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
#include <fs/vfs.h>
#include <status.h>

#define MESSAGE "hello world\n"

void kmain()
{
    init_serial();
    serial_write("Bleed Serial Output:\n");

    init_gdt();
    init_idt();
    kprintf(LOG_INFO "Physical Memory: %lluMiB\n", get_usable_pmem_size() / 1024 / 1024);
    kprintf(LOG_INFO "Highest Free PADDR: 0x%p\n", get_max_paddr());
    init_pmm();
    extend_paging();

    vfs_mount_root();

    INode_t *testfile;
    path_t testfilepath = path_from_abs("test");
    long e = vfs_create(&testfilepath, &testfile, false);
    if (e < 0) kprintf(LOG_ERROR "Failure to create file error: %d\n", status_to_string(-e));

    e = inode_write(testfile, MESSAGE, strlen(MESSAGE), 0);
    if (e < 0) kprintf(LOG_ERROR "Failure writing to file %s error: %s\n", testfile->internal_data, status_to_string(-e));

    char buffer[sizeof(MESSAGE)];
    long r = inode_read(testfile, buffer, sizeof(buffer), 0);
    if (r < 0) kprintf(LOG_ERROR "Failure reading file %s error: %s\n", testfile->internal_data, status_to_string(-e));
    buffer[r] = '\0';

    kprintf("(%s) %s", testfile->internal_data, buffer);

    for (;;) {}
    kpanic("KERNEL_FINISHED_EXECUTION");
}