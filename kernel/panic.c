#include <cpu/cpu_state.h>
#include <drivers/serial.h>
#include <stdio.h>
#include <ansii.h>

/*
    this is bad and needs to be split
        - CPU Generated Exceptions
        - Kernel Generated Exceptions

    for now RIP will always be save_cpu_state()
    not the faulty call
*/

__attribute__((noreturn))
void kpanic(const char* reason) {
    __asm__("cli");

    cpu_state_t cpu;
    save_cpu_state(&cpu);

    serial_write("\n>>> Kernel Panic <<<\n");
    serial_write("Reason: ");
    serial_write(reason);
    serial_write("\n");
    
    kprintf("%s>>> KERNEL PANIC <<<%s\n", RGB_FG(255, 0, 0), RESET);
    kprintf("%sReason: %s%s\n", RGB_FG(255, 0, 0), reason, RESET);

    dump_cpu_state(&cpu);

    kprintf("%s>>> CPU Halted <<<%s\n", RGB_FG(255, 0, 0), RESET);
    
    for(;;){
        __asm__("hlt");
    }
}