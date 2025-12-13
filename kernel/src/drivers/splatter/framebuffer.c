#include <vendor/limine/limine.h>
#include <fonts/psf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/*
    Change this to your framebuffer address and 
    thats about it!
*/

extern volatile struct limine_framebuffer_request framebuffer_request;

void* get_framebuffer_addr(){
    return framebuffer_request.response->framebuffers[0]->address;
}

uint64_t get_framebuffer_pitch(){
    return framebuffer_request.response->framebuffers[0]->pitch / 4;
}

uint64_t get_framebuffer_width(){
    return framebuffer_request.response->framebuffers[0]->width;
}

uint64_t get_framebuffer_height(){
    return framebuffer_request.response->framebuffers[0]->height;
}