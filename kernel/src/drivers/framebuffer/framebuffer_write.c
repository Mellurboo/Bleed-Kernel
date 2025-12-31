#include <stdint.h>
#include <vendor/limine_bootloader/limine.h>
#include <drivers/framebuffer/framebuffer.h>
#include <drivers/framebuffer/framebuffer_console.h>
#include <string.h>

extern volatile struct limine_framebuffer_request framebuffer_request;

/// @brief clear the top row and shift everything up one
/// @param background_colour 
static void framebuffer_scroll(fb_console_t *fb) {
    size_t row_px = fb->font->height;
    size_t scroll_px = row_px * fb->pitch;
    size_t total_px = fb->pitch * fb->height;

    memmove(
        fb->pixels,
        fb->pixels + scroll_px,
        (total_px - scroll_px) * sizeof(uint32_t)
    );

    size_t start = (fb->height - row_px) * fb->pitch;
    for (size_t i = 0; i < row_px * fb->pitch; i++)
        fb->pixels[start + i] = fb->bg;
}

/// @brief write a character to the framebuffer
/// @param font text font
/// @param c character
/// @param fg foreground colour
/// @param bg background colour
void framebuffer_put_char(fb_console_t *fb, char c) {
    if (!fb || !fb->font)
        return;

    switch (c) {
    case '\n':
        fb->cursor_x = 0;
        fb->cursor_y++;
        break;

    case '\r':
        fb->cursor_x = 0;
        break;

    case '\b':
        if (fb->cursor_x > 0)
            fb->cursor_x--;
        break;

    case '\t':
        fb->cursor_x = (fb->cursor_x + 8) & ~7;
        break;

    default: {
        if ((uint8_t)c < 0x20)
            return;

        const uint8_t *glyph =
            psf_get_glyph_font(fb->font, (uint16_t)c);
        if (!glyph)
            return;

        for (uint32_t row = 0; row < fb->font->height; row++) {
            for (uint32_t col = 0; col < fb->font->width; col++) {
                uint8_t byte =
                    glyph[row * fb->font->bytes_per_row + (col >> 3)];
                uint8_t mask = 0x80 >> (col & 7);

                size_t px = fb->cursor_x * fb->font->width + col;
                size_t py = fb->cursor_y * fb->font->height + row;

                fb->pixels[py * fb->pitch + px] =
                    (byte & mask) ? fb->fg : fb->bg;
            }
        }

        fb->cursor_x++;
        break;
    }
    }

    size_t max_cols = fb->width / fb->font->width;
    size_t max_rows = fb->height / fb->font->height;

    if (fb->cursor_x >= max_cols) {
        fb->cursor_x = 0;
        fb->cursor_y++;
    }

    if (fb->cursor_y >= max_rows) {
        framebuffer_scroll(fb);
        fb->cursor_y = max_rows - 1;
    }
}

/// @brief recursivly write characters from a string to the framebuffer
/// @param str target
void framebuffer_write_string(fb_console_t *fb, ansii_state_t *ansi, const char *str) {
    while (*str) {
        framebuffer_ansi_char(fb, ansi, *str++);
    }
}