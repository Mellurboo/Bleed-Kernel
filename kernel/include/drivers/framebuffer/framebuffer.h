#pragma once

#include <fonts/psf.h>

typedef struct tty_cursor{
    uint64_t x;
    uint64_t y;  
} tty_cursor_t;

/// @return pointer to framebuffer
void* framebuffer_get_addr();

/// @return bytes between 2 scanlines
uint64_t framebuffer_get_pitch();

/// @return framebuffer (x)
uint64_t framebuffer_get_width();

/// @return framebuffer (y)
uint64_t framebuffer_get_height();

/// @brief the terminals cursor position
/// @return (x, y) terminal cursor
tty_cursor_t cursor_get_position();

/// @brief write a character to the framebuffer
/// @param font text font
/// @param c character
/// @param fg foreground colour
/// @param bg background colour
void framebuffer_put_char(psf_font_t* font, char c, uint32_t fg, uint32_t bg);

/// @brief recursivly write characters from a string to the framebuffer
/// @param str target
void framebuffer_write_string(const char* str);

/// @brief evaluate c and track its ansii state
/// @param c target
void framebuffer_ansi_char(char c);