#ifndef PIT_H
#define PIT_H

#include <stdint.h>

#define PIT_FREQUENCY   1193182

extern volatile uint64_t timer_ticks;
void set_pit_count(unsigned count);


void sleep_ms(uint32_t ms);

#endif