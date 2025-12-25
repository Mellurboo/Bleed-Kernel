#include <stdint.h>
#include <stdio.h>
#include <ansii.h>
#include <vendor/limine_bootloader/limine.h>
#include <drivers/ps2/PS2_keyboard.h>
#include <gdt/gdt.h>
#include <idt/idt.h>
#include <string.h>
#include <mm/pmm.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <self-test/ktests.h>
#include <drivers/serial/serial.h>
#include <drivers/pic/pic.h>
#include <exec/shell/tempshell.h>
#include <drivers/pit/pit.h>
#include <fs/vfs.h>
#include <status.h>
#include <fs/archive/tar.h>
#include <sys/sleep.h>
#include <fonts/psf.h>
#include <drivers/framebuffer/framebuffer.h>
#include <sched/scheduler.h>
#include <threads/exit.h>
#include <exec/elf_load.h>
#include <tss/tss.h>
#include <panic.h>

extern volatile struct limine_module_request module_request;
extern void init_sse(void);

void initrd_load(){ 
    if (!module_request.response || module_request.response->module_count == 0){
        serial_printf("No Modules Found by booloader\n");
        return;
    }

    struct limine_file* initrd = module_request.response->modules[0];
    tar_extract(initrd->address, initrd->size);
    return;
}

// we give the kernel a task here
void scheduler_start(void) {
    uint64_t rsp;
    asm volatile("mov %%rsp, %0" : "=r"(rsp));

    sched_bootstrap((void *)rsp);
}

void splash(){
    INode_t* splash = NULL;
    path_t splash_path = vfs_path_from_abs("initrd/etc/splash.txt");

    vfs_lookup(&splash_path, &splash);
    
    size_t splash_size = vfs_filesize(splash);
    char* splash_buffer = kmalloc(splash_size + 1);

    inode_read(splash, splash_buffer, splash_size, 0);

    kprintf("%s\n", splash_buffer);
}

void load_elf_from_initrd(const char *path){
    INode_t *file = NULL;
    path_t filepath = vfs_path_from_abs(path);

    vfs_lookup(&filepath, &file);
    paddr_t cr3 = paging_create_address_space();

    uintptr_t entry;
    if (file != NULL) elf_load(file, cr3, &entry);
    sched_create_task(cr3, entry, USER_CS, USER_SS);
}

void kernel_self_test(){
    paging_test_self_test();
    pmm_test_self_test();
    pit_test_self_test();
    scheduler_test_self_test();
    vfs_test_self_test();

    framebuffer_clear(framebuffer_get_addr(0),
                        framebuffer_get_width(0),
                        framebuffer_get_height(0),
                        framebuffer_get_pitch(0),
                        0x000000);
                        
    cursor_set_position(0, 0);
}

void kmain() {
    asm volatile ("cli");
    init_sse();
    serial_init();
    pmm_init();

    reinit_paging();
    vfs_mount_root();
    initrd_load();
    psf_init("initrd/fonts/ttyfont.psf");

    gdt_init();
    idt_init();
    tss_init();
    pit_init(1000);
    pic_init(32, 40);

    scheduler_start();
    sched_create_task(read_cr3(), (uint64_t)scheduler_reap, KERNEL_CS, KERNEL_SS);
    kernel_self_test();
    load_elf_from_initrd("initrd/bin/helloworld.elf");
    asm volatile ("sti");

    kprintf(LOG_INFO "Physical Memory: %ldMiB\n", paging_get_usable_mem_size() / 1024 / 1024);
    kprintf(LOG_INFO "Highest Free PADDR: 0x%p\n", (void*)get_max_paddr());
    PS2_Keyboard_init();
    shell_start();

    for (;;){}

    ke_panic("Kernel Main Thread Died");
    return;
}
