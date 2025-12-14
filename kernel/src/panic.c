#include <stdio.h>
#include <stdint.h>
#include <ansii.h>
#include <cpu/stack_trace.h>
#include <drivers/serial/serial.h>

__attribute__((noreturn))
void ke_panic(const char* reason){
    __asm__ volatile ("cli");

    serial_write("Kernel Panic!\n");

    kprintf("\n");
    kprintf(RED_FG "===============================================\n");
    kprintf("   !!!  FATAL KERNEL EXCEPTION OCCURRED  !!!\n");
    kprintf("===============================================\n" RESET);

    kprintf(ORANGE_FG "(Kenrel Raised) EXCEPTION: %s\n", reason);

    uint64_t cur_rbp;
    __asm__ volatile ("mov %%rbp, %0" : "=r"(cur_rbp));
    print_stack_trace((uint64_t*)cur_rbp);
    kprintf("\n" LOG_INFO " SYSTEM HALTED\n" RESET);
    __asm__ volatile ("hlt");
    for(;;){}
}