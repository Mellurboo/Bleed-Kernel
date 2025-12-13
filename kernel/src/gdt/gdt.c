#include <gdt/gdt.h>
#include <ansii.h>
#include <stdint.h>
#include <stdio.h>
#include <drivers/serial/serial.h>

extern void load_gdt(void *);

struct gdt_entry_t entries[7];
struct gdt_ptr_t gdt_ptr;

void set_gdt_gate(uint8_t i, uint32_t base, uint32_t limit, uint8_t access, uint8_t flag){
    entries[i].base_low     = (base >> 0);
    entries[i].base_middle  = (base >> 16);
    entries[i].base_high    = (base >> 24);

    entries[i].limit_16     = (limit >> 0);     // 16 bits of the first limit
    entries[i].limit_8      = (limit >> 16);    // last bit of the limit

    entries[i].access       = access;
    entries[i].flags        = flag;
}

void init_gdt(){
    set_gdt_gate(0, 0, 0, 0, 0);
    set_gdt_gate(1, 0, 0xFFFFF, 0x9A, 0xA);
    set_gdt_gate(2, 0, 0xFFFFF, 0x92, 0xC);
    set_gdt_gate(3, 0, 0xFFFFF, 0xFA, 0xA);
    set_gdt_gate(4, 0, 0xFFFFF, 0xF2, 0xC);

    gdt_ptr.address = (&entries);
    gdt_ptr.length  = ((sizeof(struct gdt_entry_t) * 7) - 1);

    load_gdt(&gdt_ptr);
    serial_printf(LOG_OK "Global Descriptor Table Loaded (GDTR=%p)\n", gdt_ptr.address);
}