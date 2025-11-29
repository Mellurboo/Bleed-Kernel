#include <stdint.h>
#include <stdio.h>
#include <ascii.h>

#define PRINT_REG(name, value) \
    kprintf(#name "=%s0x%p%s\t", CYAN, (void*)(value), RESET)

typedef struct cpu_state {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rdx, rcx, rbx, rax;
    uint64_t rip, cs, rflags;
    uint64_t rsp, ss;
    uint64_t cr0, cr2, cr3, cr4, cr8;
    uint8_t vector;
} cpu_state_t;


const char* exception_name(uint8_t vector) {
    static const char* names[32] = {
        "Divide Error", "Debug", "Non-maskable Interrupt", "Breakpoint",
        "Overflow", "Bound Range Exceeded", "Invalid Opcode", "Device Not Available",
        "Double Fault", "Coprocessor Segment Overrun", "Invalid TSS", "Segment Not Present",
        "Stack Segment Fault", "General Protection Fault", "Page Fault", "Reserved",
        "x87 FPU Floating-Point Error", "Alignment Check", "Machine Check", "SIMD Floating-Point Exception",
        "Virtualization Exception", "Control Protection Exception", "Reserved", "Reserved",
        "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Security Exception", "Reserved"
    };
    return vector < 32 ? names[vector] : "Unknown";
}

__attribute__((noreturn))
void ke_exception_handler(cpu_state_t *cpu, uint8_t vector) {
    __asm__ volatile("mov %%cr0, %0" : "=r"(cpu->cr0));
    __asm__ volatile("mov %%cr2, %0" : "=r"(cpu->cr2));
    __asm__ volatile("mov %%cr3, %0" : "=r"(cpu->cr3));
    __asm__ volatile("mov %%cr4, %0" : "=r"(cpu->cr4));
    __asm__ volatile("mov %%cr8, %0" : "=r"(cpu->cr8));

    kprintf("\n%s>>> UNHANDLED KERNEL EXCEPTION <<<\n%s", RED, RESET);
    kprintf("%sException Vector: %d (%s)%s\n", RED, vector, exception_name(vector), RESET);
    PRINT_REG(RAX, cpu->rax);
    PRINT_REG(RBX, cpu->rbx);
    kprintf("\n");
    PRINT_REG(RCX, cpu->rcx);
    PRINT_REG(RDX, cpu->rdx);
    kprintf("\n");
    PRINT_REG(RSI, cpu->rsi);
    PRINT_REG(RDI, cpu->rdi);
    kprintf("\n");
    PRINT_REG(RIP, cpu->rip);
    PRINT_REG(CS, cpu->cs);
    PRINT_REG(RFLAGS, cpu->rflags);
    kprintf("\n");
    PRINT_REG(R15, cpu->r15);
    PRINT_REG(R14, cpu->r14);
    kprintf("\n");
    PRINT_REG(R13, cpu->r13);
    PRINT_REG(R12, cpu->r12);
    kprintf("\n");
    PRINT_REG(R11, cpu->r11);
    PRINT_REG(R10, cpu->r10);
    kprintf("\n");
    PRINT_REG(R9, cpu->r9);
    PRINT_REG(R8, cpu->r8);
    kprintf("\n");
    PRINT_REG(RSP, cpu->rsp);
    PRINT_REG(SS, cpu->ss);
    kprintf("\n");
    PRINT_REG(CR0, cpu->cr0);
    PRINT_REG(CR2, cpu->cr2);
    kprintf("\n");
    PRINT_REG(CR3, cpu->cr3);
    PRINT_REG(CR4, cpu->cr4);
    PRINT_REG(CR8, cpu->cr8);
    kprintf("\n");
    kprintf("%s>>> Interrupts are Disabled, CPU Halted <<<%s\n", RED, RESET);
    __asm__ volatile("cli; hlt");
}
