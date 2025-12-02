#ifndef CPU_STATE_H
#define CPU_STATE_H

#include <stdint.h>
#include <ansii.h>

typedef struct cpu_state {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rdx, rcx, rbx, rax;
    uint64_t rip, cs, rflags;
    uint64_t rsp, ss;
    uint64_t cr0, cr2, cr3, cr4, cr8;
    uint8_t vector;
} cpu_state_t;


#define PRINT_REG(name, value) \
    kprintf(#name "=%s0x%p%s\t", CYAN, (void*)(value), RESET)

void save_cpu_state(cpu_state_t* cpu);
void dump_cpu_state(cpu_state_t* cpu);

#endif