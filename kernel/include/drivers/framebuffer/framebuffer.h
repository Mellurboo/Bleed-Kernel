#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H 1

#include <lib/flanterm/impl/fb.h>
#include <lib/limine/limine.h>

struct flanterm_context *get_flanterm_context();

void fb_fill(uint8_t r, uint8_t g, uint8_t b);
void set_tty_fg_colour(uint8_t r, uint8_t g, uint8_t b);
void set_tty_bg_colour(uint8_t r, uint8_t g, uint8_t b);
#endif