#include <vendor/flanterm/impl/fb.h>
#include <vendor/flanterm/flanterm.h>
#include <vendor/flanterm/flanterm.h>
#include <vendor/limine/limine.h>
#include <stdio.h>

extern volatile struct limine_framebuffer_request framebuffer_request;

struct flanterm_context *get_flanterm_context() {
    static struct flanterm_context *ft_ctx = NULL;

    if (!ft_ctx) {
        ft_ctx = flanterm_fb_init(
            NULL,
            NULL,
            framebuffer_request.response->framebuffers[0]->address,
            framebuffer_request.response->framebuffers[0]->width,
            framebuffer_request.response->framebuffers[0]->height,
            framebuffer_request.response->framebuffers[0]->pitch,
            framebuffer_request.response->framebuffers[0]->red_mask_size,
            framebuffer_request.response->framebuffers[0]->red_mask_shift,
            framebuffer_request.response->framebuffers[0]->green_mask_size,
            framebuffer_request.response->framebuffers[0]->green_mask_shift,
            framebuffer_request.response->framebuffers[0]->blue_mask_size,
            framebuffer_request.response->framebuffers[0]->blue_mask_shift,
            NULL, NULL, NULL,
            NULL, NULL,
            NULL, NULL,
            NULL, 0, 0, 1,
            0, 0,
            0
        );
    }

    return ft_ctx;
}

void* get_framebuffer_addr(){
    return framebuffer_request.response->framebuffers[0]->address;
}