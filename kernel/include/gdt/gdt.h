#pragma once

#include <stdint.h>

struct gdt_entry_t{
    uint16_t limit_16;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t limit_8 : 4;
    uint8_t flags   : 4;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr_t{
    uint16_t length;
    void* address;
} __attribute__((packed));

/// @brief initalise the better gdt, replacing the one from LIMINE
void gdt_init();
