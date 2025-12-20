#pragma once
#include <stdint.h>
#include <cpu/io.h>

#define PIT_FREQUENCY   1193182

extern volatile uint64_t timer_ticks;

/// @brief Sets the reload divisor for PIT-0
/// @param count PIT Clock Ticks per Period
void pit_set_event_counter(uint32_t count);