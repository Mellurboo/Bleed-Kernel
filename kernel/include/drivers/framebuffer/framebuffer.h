#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H 1

#include <vendor/flanterm/impl/fb.h>
#include <vendor/limine/limine.h>

struct flanterm_context *get_flanterm_context();

void* get_fb_addr();

#endif