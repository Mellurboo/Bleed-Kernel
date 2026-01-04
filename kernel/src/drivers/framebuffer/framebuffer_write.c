#include <stdint.h>
#include <string.h>
#include <vendor/limine_bootloader/limine.h>
#include <drivers/framebuffer/framebuffer.h>
#include <drivers/framebuffer/framebuffer_console.h>
#include <mm/spinlock.h>
#include <panic.h>

extern volatile struct limine_framebuffer_request framebuffer_request;

// TODO: this wont scale well at all, change later
uint32_t fb_buffer[4096*256];

uint32_t* framebuffer_get_buffer(void) {
    return fb_buffer;
}

static inline void framebuffer_clear_row(uint32_t* buffer, size_t y, size_t pitch, uint32_t color) {
    uint32_t* row = buffer + y * pitch;
    for (size_t i = 0; i < pitch; i++)
        row[i] = color;
}

static void framebuffer_scroll_buffer(fb_console_t *fb) {
    size_t row_px = fb->font->height;

    memmove(fb_buffer,
            fb_buffer + row_px * fb->pitch,
            (fb->height - row_px) * fb->pitch * sizeof(uint32_t));

    for (size_t y = fb->height - row_px; y < fb->height; y++)
        framebuffer_clear_row(fb_buffer, y, fb->pitch, fb->bg);

    fb->cursor_y = (fb->height / fb->font->height) - 1;

    framebuffer_blit(fb_buffer, fb->pixels, fb->width, fb->height);
}

static inline void framebuffer_draw_glyph_row_mem(fb_console_t *fb, size_t x, size_t y,
                                                        const uint8_t *glyph_row, uint32_t fg, uint32_t bg) {
    if (y >= fb->height) return;
    uint32_t* dst = fb_buffer + y * fb->pitch + x;
    size_t w = fb->font->width;

    for (unsigned int byte = 0; byte < fb->font->bytes_per_row; byte++) {
        uint8_t bits = glyph_row[byte];
        for (int bit = 0; bit < 8 && (byte*8+bit) < w; bit++)
            dst[byte*8+bit] = (bits & (0x80 >> bit)) ? fg : bg;
    }
}

void framebuffer_blit(uint32_t* source, uint32_t* destination, uint32_t width, uint32_t height) {
    uint64_t* to = (uint64_t*)destination;
    uint64_t* from = (uint64_t*)source;
    for (uint64_t i = 0; i < (width * height) / 2; i++)
        *to++ = *from++;
}

static void framebuffer_render_char_mem(fb_console_t *fb, size_t row, size_t col, char c, uint32_t fg, uint32_t bg) {
    if (!c) return;
    const uint8_t *glyph = psf_get_glyph_font(fb->font, (uint16_t)c);
    if (!glyph) return;

    size_t px = col * fb->font->width;
    size_t py = row * fb->font->height;

    for (uint32_t r = 0; r < fb->font->height; r++)
        framebuffer_draw_glyph_row_mem(fb, px, py + r, &glyph[r * fb->font->bytes_per_row], fg, bg);
}

void framebuffer_put_char(fb_console_t *fb, char c) {
    size_t max_cols = fb->width / fb->font->width;
    size_t max_rows = fb->height / fb->font->height;

    switch (c) {
    case '\n': fb->cursor_x = 0; fb->cursor_y++; break;
    case '\r': fb->cursor_x = 0; break;
    case '\b': if (fb->cursor_x) fb->cursor_x--; break;
    case '\t': fb->cursor_x = (fb->cursor_x + 8) & ~7; break;
    default:
        if ((uint8_t)c < 0x20) return;
        framebuffer_render_char_mem(fb, fb->cursor_y, fb->cursor_x, c, fb->fg, fb->bg);
        fb->cursor_x++;
        break;
    }

    if (fb->cursor_x >= max_cols) {
        fb->cursor_x = 0;
        fb->cursor_y++;
    }

    if (fb->cursor_y >= max_rows)
        framebuffer_scroll_buffer(fb);
    else
        framebuffer_blit(fb_buffer, fb->pixels, fb->width, fb->height); // blit after each char
}

void framebuffer_write_string(fb_console_t *fb, ansii_state_t *ansi, const char *str, spinlock_t *framebuffer_lock) {
    while (*str)
        framebuffer_ansi_char(fb, framebuffer_lock, ansi, *str++);
}
