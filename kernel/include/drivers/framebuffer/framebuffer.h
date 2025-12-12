#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H 1

#include <vendor/flanterm/impl/fb.h>
#include <vendor/limine/limine.h>
#include <fonts/psf.h>

struct flanterm_context *get_flanterm_context();
struct flanterm_context *set_flanterm_context(psf1_font_t *font, size_t font_width, size_t font_height, bool flush);
void* get_fb_addr();

void fontwrite(psf1_font_t* font, const char* str, uint32_t fg, uint32_t bg);

#endif