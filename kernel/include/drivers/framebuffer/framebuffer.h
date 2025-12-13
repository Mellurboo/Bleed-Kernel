#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H 1

#include <fonts/psf.h>

typedef struct cursor{
    uint64_t x;
    uint64_t y;  
} cursor;

void* get_framebuffer_addr();
uint64_t get_framebuffer_pitch();

cursor get_cursor_pos();

void splatter_putc(psf_font_t* font, char c, uint32_t fg, uint32_t bg);
void splatter_write(const char* str);
void ansi_handle_char(char c);

#endif