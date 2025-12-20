#include <cpu/io.h>
#include <stdio.h>
#include <drivers/ps2/ps2_keyboard.h>
#include <drivers/pit/pit.h>
#include <drivers/pic/pic.h>
#include <ansii.h>
#include <sched/scheduler.h>

volatile uint64_t timer_ticks;
volatile uint64_t pit_countdown = 0;

/// @brief remaps the pic
/// @param master_offset remap offset for the master pic
/// @param slave_offset remap offset for the slave pic
void pic_init(int master_offset, int slave_offset){

    outb(PIC1_CMD, ICW1_INIT);
    outb(PIC2_CMD, ICW1_INIT);
    
    // Set vector offsets
    outb(PIC1_DATA, master_offset);
    outb(PIC2_DATA, slave_offset);

    outb(PIC1_DATA, 4);
    outb(PIC2_DATA, 2);

    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    outb(PIC1_DATA, 0xFC);    // kbd and timer mask
    outb(PIC2_DATA, 0xFF);    // disable all slave IRQs
}

void irq_handler(uint8_t irq, cpu_context_t *r) {
    PIC_EOI(irq);
    switch (irq) {
        case 0:
            sched_tick(r);
            timer_ticks++;
            if (pit_countdown > 0)
                pit_countdown--;
            break;
        case 1:
            PS2_Keyboard_Interrupt(irq);
            break;
        default:
            kprintf(LOG_WARN "pic sent us an unimplemented req\n");
            break;
    }
}