#include <drivers/framebuffer/framebuffer.h>
#include <lib/flanterm/impl/fb.h>

extern volatile struct limine_framebuffer_request framebuffer_request;

static inline uint32_t rgb_to_pixel(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16) | (g << 8) | b;
}

static inline void putpixel(int x, int y, uint32_t color) {
    uint32_t *fb_ptr = framebuffer_request.response->framebuffers[0]->address;
    fb_ptr[y * (framebuffer_request.response->framebuffers[0]->pitch / 4) + x] = color;
}

void fb_fill(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t colour = rgb_to_pixel(r, g, b);
    kprintf("\e[2J\e[H");
    for (int y = 0; y < framebuffer_request.response->framebuffers[0]->height; y++) {
        for (int x = 0; x < framebuffer_request.response->framebuffers[0]->width; x++) {
            putpixel(x, y, colour);
        }
    }
}