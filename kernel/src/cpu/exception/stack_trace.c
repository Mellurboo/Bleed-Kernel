#include <stdio.h>
#include <stdint.h>
#include <ansii.h>
#include <drivers/serial/serial.h>

void stack_trace_print(uint64_t *rbp) {
    kprintf("\n%sStack trace:%s\n", ORANGE_FG, RESET);
    serial_write("\nStack Trace:\n");

    for (int i = 0; i < 16 && rbp; i++) {
        if ((uint64_t)rbp < 0x1000 || ((uint64_t)rbp & 0xF)) break;

        uint64_t rip = rbp[1];
        if (!rip) break;

        kprintf("  %s0x%s%p\n", GRAY_FG, RESET, (void*)rip);
        serial_write_hex(rip);
        serial_write("\n");

        rbp = (uint64_t*)rbp[0];
    }
}