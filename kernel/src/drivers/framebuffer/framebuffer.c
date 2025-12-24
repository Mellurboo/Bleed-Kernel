#include <vendor/limine_bootloader/limine.h>
#include <drivers/framebuffer/framebuffer.h>
#include <drivers/pit/pit.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ansii.h>

extern volatile struct limine_framebuffer_request framebuffer_request;

uint32_t global_bg_color = 0x000000;

void* framebuffer_get_addr() {
    return framebuffer_request.response->framebuffers[0]->address;
}

uint64_t framebuffer_get_pitch() {
    return framebuffer_request.response->framebuffers[0]->pitch / 4;
}

uint64_t framebuffer_get_width() {
    return framebuffer_request.response->framebuffers[0]->width;
}

uint64_t framebuffer_get_height() {
    return framebuffer_request.response->framebuffers[0]->height;
}

void framebuffer_clear(uint32_t color) {
    uint32_t* fb = (uint32_t*)framebuffer_get_addr();
    uint64_t width = framebuffer_get_width();
    uint64_t height = framebuffer_get_height();
    uint64_t pitch = framebuffer_get_pitch();

    for (uint64_t y = 0; y < height; y++) {
        for (uint64_t x = 0; x < width; x++) {
            fb[y * pitch + x] = color;
        }
    }

    cursor_set_position(0, 0);
}
