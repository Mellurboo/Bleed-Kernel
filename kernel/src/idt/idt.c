#include <idt/idt.h>
#include <ansii.h>
#include <stdio.h>
#include <stdint.h>
#include <drivers/serial/serial.h>

#define DESCRIPTORS_COUNT       256
extern void* isr_stub_table[];
extern void* irq_stub_table[];

static uint8_t vectors[DESCRIPTORS_COUNT];

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

    for (uint8_t irq = 0; irq < 16; irq++) {
        set_idt_descriptor(32 + irq, irq_stub_table[irq], 0x8E);
        vectors[32 + irq] = 1;
    }

    __asm__ volatile ("lidt %0" : : "m"(idt_ptr));
    serial_printf(LOG_OK "Interrupt Descriptor Table Loaded (IDTR=%p)\n", (void*)idt_ptr.address);
}
