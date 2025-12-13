#include <stdint.h>
#include <vendor/limine/limine.h>
#include <drivers/framebuffer/framebuffer.h>

extern volatile struct limine_framebuffer_request framebuffer_request;

uint64_t cursorx = 0;
uint64_t cursory = 0;

/// @brief write a character to the framebuffer
/// @param font text font
/// @param c character
/// @param fg foreground colour
/// @param bg background colour
void splatter_putc(psf_font_t* font, char c, uint32_t fg, uint32_t bg){
    if (!font) return;

    switch (c) {
        case '\n':
            cursorx = 0;
            cursory++;
            return;
        case '\r':
            cursorx = 0;
            return;
        case '\b':
            if (cursorx > 0) cursorx--;
            return;
        case '\t':
            cursorx = (cursorx + 8) & ~7;
            return;
    }

    const uint8_t *glyph = psf_get_glyph(font, (uint16_t)c);
    if (!glyph) return;

    uint32_t *fb_ptr = (uint32_t*)get_framebuffer_addr();
    size_t pitch = framebuffer_request.response->framebuffers[0]->pitch / 4;

    for (uint32_t row = 0; row < font->height; row++){
        for (uint32_t col = 0; col < font->width; col++){
            uint8_t byte = glyph[row * font->bytes_per_row + (col >> 3)];
            uint8_t mask = 0x80 >> (col & 7);
            uint32_t color = (byte & mask) ? fg : bg;

            size_t px = cursorx * font->width + col;
            size_t py = cursory * font->height + row;
            fb_ptr[py * pitch + px] = color;
        }
    }

    cursorx++;
    if (cursorx >= framebuffer_request.response->framebuffers[0]->width / font->width){
        cursorx = 0;
        cursory++;
    }
}

/// @brief write text to the framebuffer
/// @param str string
void splatter_write(const char* str) {
    while (*str) {
        ansi_handle_char(*str++);
    }
}