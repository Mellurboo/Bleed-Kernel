#include <cpu/io.h>
#include <stdio.h>
#include <drivers/ps2/ps2_keyboard.h>
#include <drivers/pit/pit.h>
#include <ansii.h>
#include <sched/scheduler.h>

#define PIC1        0x20    // Master PIC
#define PIC2        0xA0    // Slave PIC
#define PIC1_CMD    PIC1
#define PIC1_DATA   (PIC1 + 1)
#define PIC2_CMD    PIC2
#define PIC2_DATA   (PIC2 + 1)

#define ICW1_INIT   0x11
#define ICW4_8086   0x01

volatile uint64_t timer_ticks;
volatile uint64_t pit_countdown = 0;

/// @brief remaps the pic
/// @param master_offset remap offset for the master pic
/// @param slave_offset remap offset for the slave pic
void init_pic(int master_offset, int slave_offset){

    outb(PIC1_CMD, ICW1_INIT);
    outb(PIC2_CMD, ICW1_INIT);
    
    // Set vector offsets
    outb(PIC1_DATA, master_offset);
    outb(PIC2_DATA, slave_offset);

    outb(PIC1_DATA, 4);
    outb(PIC2_DATA, 2);

    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    outb(PIC1_DATA, 0xFC);    // kbd and timer
    outb(PIC2_DATA, 0xFF);    // disable all slave IRQs
}

void pic_eoi(uint8_t irq){
    if (irq >= 8) {
        outb(PIC2_CMD, 0x20);
    }else{
        outb(PIC1_CMD, 0x20);
    }
}
//extern void irq_handler(uint8_t irq, regs_t *r) <- we will need this later, but for now ill leave it here
void irq_handler(uint8_t irq, cpu_context_t *r) {
    pic_eoi(irq);
    switch (irq) {
        case 0:
            scheduler_tick(r);
            timer_ticks++;
            if (pit_countdown > 0)
                pit_countdown--;
            break;
        case 1:
            ps2_keyboard_irq(irq);
            break;
        default:
            kprintf(LOG_WARN "pic sent us an unimplemented req\n");
            break;
    }
}