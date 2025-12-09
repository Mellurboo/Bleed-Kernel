/*
    This file has been dramatically changed:
        Before, there was an issue that made kernel panics
        useless, becuase RIP would always point to the 
        save cpu registers function, this reform
        aims to completly fix that silly issue
*/

#include <stdint.h>
#include <stdio.h>
#include <ansii.h>
#include <panic.h>
#include <cpu/cpu_state.h>

struct isr_stackframe {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t vector;
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
} __attribute__((packed));

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


void print_stack_trace(uint64_t *rbp) {
    kprintf("\n%sStack trace:%s\n", RED_FG, RESET);
    serial_write("\nStack Trace:\n");

    for (int i = 0; i < 16 && rbp; i++) {
        // Sanity check: RBP should be aligned and not null
        if ((uint64_t)rbp < 0x1000 || ((uint64_t)rbp & 0xF)) break;

        uint64_t rip = rbp[1];
        if (!rip) break;

        kprintf("  0x%p\n", (void*)rip);
        serial_write_hex(rip);
        serial_write("\n");

        rbp = (uint64_t*)rbp[0];
    }
}

extern void ke_exception_handler(void *frame){
    __asm__ volatile ("cli");
    struct isr_stackframe *f = (struct isr_stackframe *)frame;

    uint64_t vector = f->vector;
    uint64_t err    = f->error_code;
    uint64_t rip    = f->rip;
    uint64_t cs     = f->cs;
    uint64_t rflags = f->rflags;

    serial_write("Kernel Panic!\n");
    serial_write_hex(vector);
    serial_write("\n");
    serial_write_hex(f->rip);

    kprintf("\n");
    kprintf(RED_FG "===============================================\n");
    kprintf("   !!!  FATAL KERNEL EXCEPTION OCCURRED  !!!\n");
    kprintf("===============================================\n" FG_RESET);

    kprintf(ORANGE_FG " EXCEPTION: %s\n" FG_RESET, exception_name(vector));
    kprintf(RED_FG " VECTOR: %llu   ERROR: 0x%p\n" FG_RESET, vector, err);

    kprintf("\n" YELLOW_FG " CPU STATE:" FG_RESET "\n");
    kprintf(" RIP: " RED_FG "0x%p" FG_RESET "   CS: 0x%p   RFLAGS: 0x%p\n", rip, cs, rflags);

    kprintf("\n" BLUE_FG " Registers:" FG_RESET "\n");
    kprintf(" RAX: %s0x%p%s  RBX: %s0x%p%s  RCX: %s0x%p%s  RDX: %s0x%p%s\n",
            BLUE_FG, f->rax, RESET, BLUE_FG, f->rbx, RESET, BLUE_FG, f->rcx, RESET, BLUE_FG, f->rdx, RESET);

    kprintf(" RSI: %s0x%p%s  RDI: %s0x%p%s  RBP: %s0x%p%s\n",
            BLUE_FG, f->rsi, RESET, BLUE_FG, f->rdi, RESET, BLUE_FG, f->rbp, RESET);

    kprintf(" R8 : %s0x%p%s  R9 : %s0x%p%s  R10: %s0x%p%s  R11: %s0x%p%s\n",
            BLUE_FG, f->r8, RESET, BLUE_FG, f->r9, RESET, BLUE_FG, f->r10, RESET, BLUE_FG, f->r11, RESET);

    kprintf(" R12: %s0x%p%s  R13: %s0x%p%s  R14: %s0x%p%s  R15: %s0x%p%s\n",
            BLUE_FG, f->r12, RESET, BLUE_FG, f->r13, RESET, BLUE_FG, f->r14, RESET, BLUE_FG, f->r15, RESET);

    if (vector == 14){
        kprintf("\n" RED_FG " PAGE FAULT DETAILS\n" FG_RESET);
        uint64_t cr2 = 0; 
        __asm__ volatile ("mov %%cr2, %0" : "=r"(cr2));

        kprintf("  Faulting address: " RED_FG "0x%p" FG_RESET "\n", cr2);
        kprintf("  Error code: 0x%p\n", err);
        kprintf("   P  = %u\n", (unsigned)((err >> 0) & 1));
        kprintf("   W/R= %u\n", (unsigned)((err >> 1) & 1));
        kprintf("   U/S= %u\n", (unsigned)((err >> 2) & 1));
        kprintf("   RSV= %u\n", (unsigned)((err >> 3) & 1));
        kprintf("   I/D= %u\n", (unsigned)((err >> 4) & 1));
    }

    uint64_t cur_rbp;
    __asm__ volatile ("mov %%rbp, %0" : "=r"(cur_rbp));
    print_stack_trace((uint64_t*)cur_rbp);
    kprintf("\n" RED_FG " SYSTEM HALTED\n" FG_RESET);
    __asm__ volatile ("hlt");
    for(;;){}
}
