#include <vendor/limine/limine.h>
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