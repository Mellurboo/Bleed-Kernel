#ifndef PIC_H
#define PIC_H

extern volatile uint64_t pit_countdown;

/// @brief remaps the pic
/// @param master_offset remap offset for the master pic
/// @param slave_offset remap offset for the slave pic
void init_pic(int master_offset, int slave_offset);
void pic_eoi(uint8_t irq);

#endif