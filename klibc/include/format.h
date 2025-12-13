#ifndef FORMAT_H
#define FORMAT_H

#include <stdint.h>

char* utoa_base(uint64_t value, int base, int uppercase);
char* itoa_signed(int64_t value);

#endif