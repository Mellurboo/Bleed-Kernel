#include <x86_64/cpu/cpu_state.h>
#include <stdio.h>
#include <ansii.h>

__attribute__((noreturn))
void kpanic(const char* reason) {
    __asm__("cli");

    cpu_state_t cpu;
    save_cpu_state(&cpu);

    kprintf("%s>>> KERNEL PANIC <<<%s\n", RED, RESET);
    kprintf("%sReason: %s%s\n", RED, reason, RESET);

    dump_cpu_state(&cpu);

    kprintf("%s>>> CPU Halted <<<%s\n", RED, RESET);
    
    for(;;){
        __asm__("hlt");
    }

    __builtin_unreachable();
}