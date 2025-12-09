#include <stdint.h>
#include <stdio.h>
#include <ansii.h>
#include <lib/limine/limine.h>
#include <gdt/gdt.h>
#include <idt/idt.h>
#include <string.h>
#include <panic.h>
#include <mm/pmm.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <drivers/serial.h>
#include <drivers/framebuffer/framebuffer.h>
#include <fs/vfs.h>
#include <status.h>
#include <fs/fsutils.h>
#include <fs/archive/tar.h>

extern volatile struct limine_module_request module_request;
void load_initrd(){ 
    if (!module_request.response || module_request.response->module_count == 0){
        kprintf(LOG_ERROR "No Modules loaded by Bootloader\n");
        return;
    }

    struct limine_file* initrd = module_request.response->modules[0];
    tar_extract(initrd->address, initrd->size);
    return;
}

void splash(){
    INode_t* splash = NULL;
    path_t splash_path = path_from_abs("initrd/resources/splash.txt");

    vfs_lookup(&splash_path, &splash);
    
    size_t splash_size = vfs_filesize(splash);
    char* splash_buffer = kmalloc(splash_size + 1);

    inode_read(splash, splash_buffer, splash_size, 0);

    kprintf("%s%s%s\n", RGB_FG(200, 69, 69), splash_buffer, RESET);
}

void kmain() {
    init_pmm();
    init_serial();
    serial_write("Bleed Serial Output:\n");

    init_gdt();
    init_idt();
    kprintf(LOG_INFO "Physical Memory: %lluMiB\n", get_usable_pmem_size() / 1024 / 1024);
    kprintf(LOG_INFO "Highest Free PADDR: 0x%p\n", get_max_paddr());
    extend_paging();
    vfs_mount_root();
    load_initrd();
    list_directory("initrd");

    splash();

    for (;;){}
    kpanic("KERNEL_FINISHED_EXECUTION");
}