#include <vendor/limine_bootloader/limine.h>
#include <drivers/framebuffer/framebuffer.h>
#include <drivers/pit/pit.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ansii.h>

extern volatile struct limine_framebuffer_request framebuffer_request;

uint32_t global_bg_color = 0x000000;

void* framebuffer_get_addr(int idx) {
    return framebuffer_request.response->framebuffers[idx]->address;
}

uint64_t framebuffer_get_pitch(int idx) {
    return framebuffer_request.response->framebuffers[idx]->pitch / 4;
}

uint64_t framebuffer_get_width(int idx) {
    return framebuffer_request.response->framebuffers[idx]->width;
}

uint64_t framebuffer_get_height(int idx) {
    return framebuffer_request.response->framebuffers[idx]->height;
}

void framebuffer_clear(uint32_t *pixels, uint64_t width, uint64_t height, uint64_t pitch, uint32_t colour) {
    for (uint64_t y = 0; y < height; y++) {
        for (uint64_t x = 0; x < width; x++) {
            pixels[y * pitch + x] = colour;
        }
    }
}
