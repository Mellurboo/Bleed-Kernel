#include <vendor/flanterm/impl/fb.h>
#include <vendor/flanterm/flanterm.h>
#include <vendor/flanterm/flanterm.h>
#include <vendor/limine/limine.h>
#include <stdbool.h>
#include <fonts/psf.h>
#include <stdio.h>

extern volatile struct limine_framebuffer_request framebuffer_request;

static struct flanterm_context *ft_ctx = NULL;

struct flanterm_context *set_flanterm_context(psf1_font_t *font, size_t font_width, size_t font_height) {
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    if (!ft_ctx){
        ft_ctx = flanterm_fb_init(
        NULL,
        NULL,
        fb->address,
        fb->width,
        fb->height,
        fb->pitch,
        fb->red_mask_size,
        fb->red_mask_shift,
        fb->green_mask_size,
        fb->green_mask_shift,
        fb->blue_mask_size,
        fb->blue_mask_shift,
        NULL, NULL, NULL,
        NULL, NULL,
        NULL, NULL,
        (void *)font, font_width, font_height, 1,
        0, 0,
        0
        );
    }
    return ft_ctx;
}

struct flanterm_context *get_flanterm_context() {
    return ft_ctx;
}

void* get_framebuffer_addr(){
    return framebuffer_request.response->framebuffers[0]->address;
}


unsigned long cursorx = 0;
unsigned long cursory = 0;

void flanterm_putc(psf1_font_t* font, char c, uint32_t fg, uint32_t bg){
    if (!font) return;

    const uint8_t* glyph = psf_get_glyph(font, (uint16_t)c);
    if (!glyph) return;

    uint32_t *fb_ptr = (uint32_t *)get_framebuffer_addr();
    size_t pitch = framebuffer_request.response->framebuffers[0]->pitch / 4; // assuming 32-bit pixels

    for (size_t row = 0; row < font->charsize; row++){
        uint8_t row_data = glyph[row];
        for (size_t col = 0; col < 8; col++){
            uint32_t color = (row_data & (1 << (7 - col))) ? fg : bg;
            size_t px = cursorx * 8 + col;
            size_t py = cursory * font->charsize + row;
            fb_ptr[py * pitch + px] = color;
        }
    }

    cursorx++;
    if (cursorx >= framebuffer_request.response->framebuffers[0]->width / 8){
        cursorx = 0;
        cursory++;
    }
}

void fontwrite(psf1_font_t* font, const char* str, uint32_t fg, uint32_t bg){
    while (*str){
        if (*str == '\n'){
            cursorx = 0;
            cursory++;
        } else {
            flanterm_putc(font, *str, fg, bg);
        }
        str++;
    }
}