#include <cpu/cpu_state.h>
#include <stdio.h>

void save_cpu_state(cpu_state_t* cpu) {
    __asm__ volatile(
        "mov %%rax, %0\n\t"
        "mov %%rbx, %1\n\t"
        "mov %%rcx, %2\n\t"
        "mov %%rdx, %3\n\t"
        "mov %%rbp, %4\n\t"
        "mov %%rsi, %5\n\t"
        "mov %%rdi, %6\n\t"
        "mov %%r8, %7\n\t"
        "mov %%r9, %8\n\t"
        "mov %%r10, %9\n\t"
        "mov %%r11, %10\n\t"
        "mov %%r12, %11\n\t"
        "mov %%r13, %12\n\t"
        "mov %%r14, %13\n\t"
        "mov %%r15, %14\n\t"
        : "=m"(cpu->rax), "=m"(cpu->rbx), "=m"(cpu->rcx), "=m"(cpu->rdx),
          "=m"(cpu->rbp), "=m"(cpu->rsi), "=m"(cpu->rdi),
          "=m"(cpu->r8),  "=m"(cpu->r9), "=m"(cpu->r10), "=m"(cpu->r11),
          "=m"(cpu->r12), "=m"(cpu->r13), "=m"(cpu->r14), "=m"(cpu->r15)
        :
        : "memory"
    );

    __asm__ volatile("mov %%rsp, %0" : "=m"(cpu->rsp));
    __asm__ volatile("lea (%%rip), %%rax\n\tmov %%rax, %0" : "=m"(cpu->rip));
    __asm__ volatile("pushfq; popq %0" : "=m"(cpu->rflags));

    __asm__ volatile("mov %%cs, %0" : "=r"(cpu->cs));
    __asm__ volatile("mov %%ss, %0" : "=r"(cpu->ss));

    __asm__ volatile("mov %%cr0, %0" : "=r"(cpu->cr0));
    __asm__ volatile("mov %%cr2, %0" : "=r"(cpu->cr2));
    __asm__ volatile("mov %%cr3, %0" : "=r"(cpu->cr3));
    __asm__ volatile("mov %%cr4, %0" : "=r"(cpu->cr4));
    __asm__ volatile("mov %%cr8, %0" : "=r"(cpu->cr8));
}

void dump_cpu_state(cpu_state_t* cpu) {
    PRINT_REG(RAX, cpu->rax);
    PRINT_REG(RBX, cpu->rbx);
    PRINT_REG(RCX, cpu->rcx);
    kprintf("\n");
    PRINT_REG(RDX, cpu->rdx);
    PRINT_REG(RSI, cpu->rsi);
    PRINT_REG(RDI, cpu->rdi);
    kprintf("\n");
    PRINT_REG(RBP, cpu->rbp);
    PRINT_REG(RSP, cpu->rsp);
    kprintf("\n");

    PRINT_REG(R8, cpu->r8);
    PRINT_REG(R9, cpu->r9);
    PRINT_REG(R10, cpu->r10);
    kprintf("\n");
    PRINT_REG(R11, cpu->r11);
    PRINT_REG(R12, cpu->r12);
    PRINT_REG(R13, cpu->r13);
    kprintf("\n");
    PRINT_REG(R14, cpu->r14);
    PRINT_REG(R15, cpu->r15);
    kprintf("\n");

    PRINT_REG(RIP, cpu->rip);
    PRINT_REG(CS, cpu->cs);
    PRINT_REG(RFLAGS, cpu->rflags);
    PRINT_REG(SS, cpu->ss);
    kprintf("\n");

    PRINT_REG(CR0, cpu->cr0);
    PRINT_REG(CR2, cpu->cr2);
    PRINT_REG(CR3, cpu->cr3);
    kprintf("\n");
    PRINT_REG(CR4, cpu->cr4);
    PRINT_REG(CR8, cpu->cr8);
    kprintf("\n");
}
