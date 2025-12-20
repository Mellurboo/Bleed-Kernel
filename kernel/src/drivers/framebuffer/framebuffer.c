#include <vendor/limine_bootloader/limine.h>
#include <fonts/psf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

extern volatile struct limine_framebuffer_request framebuffer_request;

/// @return pointer to framebuffer
void* framebuffer_get_addr(){
    return framebuffer_request.response->framebuffers[0]->address;
}

/// @return bytes between 2 scanlines
uint64_t framebuffer_get_pitch(){
    return framebuffer_request.response->framebuffers[0]->pitch / 4;
}

/// @return framebuffer (x)
uint64_t framebuffer_get_width(){
    return framebuffer_request.response->framebuffers[0]->width;
}

/// @return framebuffer (y)
uint64_t framebuffer_get_height(){
    return framebuffer_request.response->framebuffers[0]->height;
}