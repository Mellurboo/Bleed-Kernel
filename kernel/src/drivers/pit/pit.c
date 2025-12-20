#include <stdint.h>
#include <cpu/io.h>
#include <drivers/pit/pit.h>

/// @brief Current countdown value of PIT-0
/// @return PIT fires until countdown is 0
uint32_t pit_read_interval_remaining(){
    asm volatile("cli");
    unsigned pit_count = 0;

	outb(0x43, 0x00);
	
	pit_count = inb(0x40);		    // Low byte
	pit_count |= inb(0x40)<<8;		// High byte
	
	return pit_count;
}

/// @brief Sets the reload divisor for PIT-0
/// @param count PIT Clock Ticks per Period
void pit_set_event_counter(uint32_t count) {
    asm volatile("cli");
	
	outb(0x40,count&0xFF);		    // Low byte
	outb(0x40,(count&0xFF00)>>8);	// High byte
	return;
}