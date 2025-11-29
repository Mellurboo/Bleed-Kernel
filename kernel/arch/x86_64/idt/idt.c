#include <x86_64/idt/idt.h>
#include <ascii.h>
#include <stdio.h>
#include <stdint.h>

#define DESCRIPTORS_COUNT       256
extern void* isr_stub_table[];

static uint8_t vectors[DESCRIPTORS_COUNT];

typedef struct {
    uint16_t    offset16;   // lower 0-15
    uint16_t    selector;   // the segment selector for CS
    uint8_t     ist;        // interupt stack table offset
    uint8_t     attributes; // also holds the type 
    uint16_t    offset32;   // higher 16-31
    uint32_t    offset64;   // higher 32-63
    uint32_t    zero;       // that top reserved bit
} __attribute__((packed)) idt_entry_t;

/// @brief idt pointer
typedef struct {
    uint16_t    limit;
    uint64_t    address;
} __attribute__((packed)) idt_ptr_t;

__attribute__((aligned(0x10)))
static idt_entry_t idt[DESCRIPTORS_COUNT];
static idt_ptr_t idt_ptr;

void set_idt_descriptor(uint8_t vector, void* isr, uint8_t flags){
    idt_entry_t* descriptor = &idt[vector];

    descriptor->offset16    = (uint64_t)isr & 0xFFFF;
    descriptor->selector    = 0x08;
    descriptor->ist         = 0;
    descriptor->attributes  = flags;
    descriptor->offset32    = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->offset64    = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->zero        = 0;
}

void init_idt(){
    idt_ptr.address = (uintptr_t)&idt[0];
    idt_ptr.limit = (uint16_t)sizeof(idt_entry_t) * DESCRIPTORS_COUNT - 1;

    for (uint8_t vector = 0; vector < 32; vector++){
        set_idt_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = 1;
    }
    
    __asm__ volatile ("lidt %0" : : "m"(idt_ptr));
    kprintf(LOG_OK "Interrupt Descriptor Table Loaded (IDTR=%p)\n", idt_ptr.address);
    __asm__ volatile ("sti");
    kprintf(LOG_OK "Interrupts Enabled!\n");
}