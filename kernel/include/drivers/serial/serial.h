#ifndef SERIAL_H
#define SERIAL_H

int init_serial();
void serial_write(const char* str);
void serial_write_hex(uint64_t value);
void serial_printf(const char* fmt, ...);

#endif