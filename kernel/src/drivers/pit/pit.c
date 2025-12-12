#include <stdint.h>
#include <cpu/io.h>
#include <drivers/pit/pit.h>

unsigned read_pit_count(){
    asm volatile("cli");
    unsigned pit_count = 0;

	outb(0x43, 0x00);
	
	pit_count = inb(0x40);		    // Low byte
	pit_count |= inb(0x40)<<8;		// High byte
	
	return pit_count;
}

void set_pit_count(unsigned count) {
    asm volatile("cli");
	
	outb(0x40,count&0xFF);		    // Low byte
	outb(0x40,(count&0xFF00)>>8);	// High byte
	return;
}