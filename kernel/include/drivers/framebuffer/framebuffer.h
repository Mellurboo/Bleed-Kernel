#pragma once

#include <fonts/psf.h>
#include <drivers/framebuffer/framebuffer_console.h>
#include <mm/spinlock.h>

typedef struct {
    uint32_t fg;
    uint32_t bg;
    int esc;
    int csi;
    int params[16];
    int param_count;
    int substate;
    int subparams[4];
} ansii_state_t;


/// @return pointer to framebuffer
void* framebuffer_get_addr(int idx);

/// @return bytes between 2 scanlines
uint64_t framebuffer_get_pitch(int idx);

/// @return framebuffer (x)
uint64_t framebuffer_get_width(int idx);

/// @return framebuffer (y)
uint64_t framebuffer_get_height(int idx);

/// @brief write a character to the framebuffer
/// @param font text font
/// @param c character
/// @param fg foreground colour
/// @param bg background colour
void framebuffer_put_char(fb_console_t *fb, char c);

/// @brief recursivly write characters from a string to the framebuffer
/// @param str target
void framebuffer_write_string(
    fb_console_t *fb,
    ansii_state_t *ansi,
    const char *str,
    spinlock_t *framebuffer_lock
);

/// @brief evaluate c and track its ansii state
/// @param c target
void framebuffer_ansi_char(fb_console_t *fb, spinlock_t *framebuffer_lock, ansii_state_t *st, char c);

/// @brief clear the framebuffer
/// @param color bg colour to clear with
void framebuffer_clear(uint32_t *pixels, uint64_t width, uint64_t height, uint64_t pitch, uint32_t colour);

void lockinit();