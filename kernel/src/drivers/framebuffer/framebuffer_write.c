#include <stdint.h>
#include <vendor/limine_bootloader/limine.h>
#include <drivers/framebuffer/framebuffer.h>
#include <string.h>

extern volatile struct limine_framebuffer_request framebuffer_request;

tty_cursor_t tty_cursor = {
    .x = 0,
    .y = 0
};

/// @brief the terminals cursor position
/// @return (x, y) terminal cursor
tty_cursor_t cursor_get_position() {
    return tty_cursor;
}

/// @brief set the tty cursor position
/// @param x new x
/// @param y new y
/// @return new cursor position
tty_cursor_t cursor_set_position(int x, int y){
    tty_cursor.x = x; 
    tty_cursor.y = y; 
    return tty_cursor;
}

/// @brief clear the top row and shift everything up one
/// @param background_colour 
static void framebuffer_scroll(uint32_t *pixels, uint64_t height, uint64_t pitch, uint32_t colour, psf_font_t *font) {
    size_t row_px = font->height;
    size_t scroll_px = row_px * pitch;

    size_t total_px = pitch * height;

    memmove(
        pixels,
        pixels + scroll_px,
        (total_px - scroll_px) * sizeof(uint32_t)
    );

    size_t start = (height - row_px) * pitch;
    for (size_t i = 0; i < row_px * pitch; i++) {
        pixels[start + i] = colour;
    }
}

/// @brief write a character to the framebuffer
/// @param font text font
/// @param c character
/// @param fg foreground colour
/// @param bg background colour
void framebuffer_put_char(uint32_t *pixels, uint64_t pitch, psf_font_t* font, char c, uint32_t fg, uint32_t bg){
    if (!font) return;

    tty_cursor_t cursor = cursor_get_position();

    switch (c) {
        case '\n':
            cursor.x = 0;
            cursor.y++;
            break;
        case '\r':
            cursor.x = 0;
            break;
        case '\b':
            if (cursor.x > 0) cursor.x--;
            break;
        case '\t':
            cursor.x = (cursor.x + 8) & ~7;
            break;
        default:
            if ((uint8_t)c < 0x20) return;

            const uint8_t *glyph = psf_get_glyph_font(font, (uint16_t)c);
            if (!glyph) return;

            for (uint32_t row = 0; row < font->height; row++){
                for (uint32_t col = 0; col < font->width; col++){
                    uint8_t byte = glyph[row * font->bytes_per_row + (col >> 3)];
                    uint8_t mask = 0x80 >> (col & 7);
                    uint32_t color = (byte & mask) ? fg : bg;

                    size_t px = cursor.x * font->width + col;
                    size_t py = cursor.y * font->height + row;
                    pixels[py * pitch + px] = color;
                }
            }

            cursor.x++;
            break;
    }

    // Wrap
    size_t max_cols = framebuffer_request.response->framebuffers[0]->width / font->width;
    size_t max_rows = framebuffer_request.response->framebuffers[0]->height / font->height;

    if (cursor.x >= max_cols) {
        cursor.x = 0;
        cursor.y++;
    }

    // i was going to make it so the user could scroll here with 
    // page up and page down but i dont think that is appropriate right now :(
    if (cursor.y >= max_rows) {
        framebuffer_scroll((uint32_t*)framebuffer_get_addr(0),
            framebuffer_get_height(0), 
            framebuffer_get_pitch(0), 
            0x000000, 
            psf_get_current_font());
        cursor.y = max_rows - 1;
    }

    // Update global cursor
    tty_cursor = cursor;
}

/// @brief recursivly write characters from a string to the framebuffer
/// @param str target
void framebuffer_write_string(const char* str) {
    while (*str) {
        framebuffer_ansi_char(*str++);
    }
}