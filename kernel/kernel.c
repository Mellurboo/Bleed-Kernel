#include <stdint.h>
#include <stdio.h>
#include <ansii.h>
#include <vendor/limine_bootloader/limine.h>
#include <gdt/gdt.h>
#include <idt/idt.h>
#include <string.h>
#include <mm/pmm.h>
#include <mm/heap.h>
#include <mm/paging.h>
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
#include <panic.h>

extern volatile struct limine_module_request module_request;
extern void init_sse(void);
extern void init_pit(uint32_t freq);

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
    path_t splash_path = vfs_path_from_abs("initrd/resources/splash.txt");

    vfs_lookup(&splash_path, &splash);
    
    size_t splash_size = vfs_filesize(splash);
    char* splash_buffer = kmalloc(splash_size + 1);

    inode_read(splash, splash_buffer, splash_size, 0);

    kprintf("%s\n", splash_buffer);
}

void task_a(void) {
    exit();
}

void kmain() {
    serial_init();
    pmm_init();
    reinit_paging();

    asm volatile ("cli");
    gdt_init();
    idt_init();
    init_pit(100);
    pic_init(32, 40);

    scheduler_start();
    sched_create_task(scheduler_reap, KERNEL_TASK);
    sched_create_task(task_a, USER_TASK);
    asm volatile ("sti");

    init_sse();

    vfs_mount_root();
    initrd_load();

    psf_init("initrd/resources/ttyfont.psf");
    
    kprintf(LOG_INFO "Physical Memory: %ldMiB\n", paging_get_usable_mem_size() / 1024 / 1024);
    kprintf(LOG_INFO "Highest Free PADDR: 0x%p\n", (void*)get_max_paddr());
    splash();
    shell_start();

    for (;;){}

    return;
}
