#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H 1

#include <fonts/psf.h>

void* get_framebuffer_addr();
void splatter_putc(psf_font_t* font, char c, uint32_t fg, uint32_t bg);
void splatter_write(const char* str);
void ansi_handle_char(char c);

#endif