/*
    this can be really funny on UEFI, and by that i mean it may cause the system
    not to boot :/
    for now i only want to write to serial but maybe recv later?
*/
#include <cpu/io.h>
#include <stdint.h>

#define PORT_COM1   0x3F8
#define SERIAL_TIMEOUT 100000

static int serial_available = 0;

static int is_serial_transmit_empty(){
    return inb(PORT_COM1 + 5) & 0x20;
}

int init_serial(){
    outb(PORT_COM1 + 4, 0x0F);
    uint8_t test = inb(PORT_COM1 + 4);
    if (test != 0x0F) {
        serial_available = 0;
        return 1; // COM1 not available
    }

    outb(PORT_COM1 + 1, 0x00);
    outb(PORT_COM1 + 3, 0x80);
    outb(PORT_COM1 + 0, 0x03);
    outb(PORT_COM1 + 1, 0x00);
    outb(PORT_COM1 + 3, 0x03);
    outb(PORT_COM1 + 2, 0xC7);
    outb(PORT_COM1 + 4, 0x0B);
    outb(PORT_COM1 + 4, 0x1E);
    outb(PORT_COM1 + 0, 0xAE);

    if (inb(PORT_COM1 + 0) != 0xAE){
        serial_available = 0;
        return 1; // COM1 failed test
    }

    outb(PORT_COM1 + 4, 0x0F);
    serial_available = 1;
    return 0;
}

void serial_write_char(char c){
    if (!serial_available) return;

    uint32_t timeout = SERIAL_TIMEOUT;
    while (!is_serial_transmit_empty() && timeout--) { }
    if (timeout == 0) return;

    outb(PORT_COM1, c);
}

void serial_write(const char* str) {
    if (!serial_available) return;

    while (*str) {
        if (*str == '\n') serial_write_char('\r');
        serial_write_char(*str++);
    }
}

void serial_write_hex(uint64_t value){
    if (!serial_available) return;

    char buffer[17];
    const char* hex = "0123456789ABCDEF";

    for (int i = 0; i < 16; i++){
        buffer[15-i] = hex[value & 0xF];
        value >>= 4;
    }
    buffer[16] = '\0';
    serial_write("0x");
    serial_write(buffer);
}