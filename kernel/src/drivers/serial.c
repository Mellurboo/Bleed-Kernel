// for now i only want to write to serial but maybe recv later?

#include <stdio.h>
#include <stdint.h>
#include <cpu/io.h>
#include <ansii.h>
#include <drivers/serial.h>

#define PORT_COM1   0x3F8

static int is_serial_transmit_empty(){
    return inb(PORT_COM1 + 5) & 0x20;
}

int init_serial(){
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
        kprintf(LOG_ERROR "Failure to initialise serial port (COM1 %s)\n", PORT_COM1);
        return 1;   // serial is "faulty"?
    }

    outb(PORT_COM1 + 4, 0x0F);
    return 0;
}

void serial_write_char(char c){
    while (!is_serial_transmit_empty()) { }
    outb(PORT_COM1, c);
}

void serial_write(const char* str) {
    while (*str) {
        if (*str == '\n')
            serial_write_char('\r');
        serial_write_char(*str++);
    }
}

void serial_write_hex(uint64_t value){
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