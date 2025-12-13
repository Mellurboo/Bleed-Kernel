#include <stdint.h>
#include <vendor/limine/limine.h>
#include <drivers/framebuffer/framebuffer.h>
#include <string.h>

extern volatile struct limine_framebuffer_request framebuffer_request;

cursor tty_cursor = {
    .x = 0,
    .y = 0
};

cursor get_cursor_pos(){
    return tty_cursor;
}

static void tty_scroll(uint32_t bg) {
    psf_font_t *font = get_tty_font();
    uint32_t *fb = (uint32_t*)get_framebuffer_addr();
    uint64_t fb_pitch = get_framebuffer_pitch();
    uint64_t fb_height = get_framebuffer_height();

    size_t row_px = font->height;
    size_t scroll_px = row_px * fb_pitch;

    size_t total_px = fb_pitch * fb_height;

    memmove(
        fb,
        fb + scroll_px,
        (total_px - scroll_px) * sizeof(uint32_t)
    );

    /* Clear the last text row */
    size_t start = (fb_height - row_px) * fb_pitch;
    for (size_t i = 0; i < row_px * fb_pitch; i++) {
        fb[start + i] = bg;
    }
}

/// @brief write a character to the framebuffer
/// @param font text font
/// @param c character
/// @param fg foreground colour
/// @param bg background colour
void splatter_putc(psf_font_t* font, char c, uint32_t fg, uint32_t bg){
    if (!font) return;

    cursor cursor = get_cursor_pos();

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

            const uint8_t *glyph = psf_get_glyph(font, (uint16_t)c);
            if (!glyph) return;

            uint32_t *fb_ptr = (uint32_t*)get_framebuffer_addr();
            size_t pitch = framebuffer_request.response->framebuffers[0]->pitch / 4;

            for (uint32_t row = 0; row < font->height; row++){
                for (uint32_t col = 0; col < font->width; col++){
                    uint8_t byte = glyph[row * font->bytes_per_row + (col >> 3)];
                    uint8_t mask = 0x80 >> (col & 7);
                    uint32_t color = (byte & mask) ? fg : bg;

                    size_t px = cursor.x * font->width + col;
                    size_t py = cursor.y * font->height + row;
                    fb_ptr[py * pitch + px] = color;
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
        tty_scroll(bg);
        cursor.y = max_rows - 1;
    }

    // Update global cursor
    tty_cursor = cursor;
}

/// @brief write text to the framebuffer
/// @param str string
void splatter_write(const char* str) {
    while (*str) {
        ansi_handle_char(*str++);
    }
}