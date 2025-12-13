#include <stdint.h>
#include <string.h>

#include <drivers/framebuffer/framebuffer.h>

typedef struct {
    uint32_t fg;
    uint32_t bg;
    int esc;
    int csi;
    int params[8];
    int param_count;
} ansii_state_t;

ansii_state_t ansi = {
    .fg = 0xFFFFFFFF,
    .bg = 0x00000000
};

static uint32_t ansi_basic_colours[8] = {
    0xFF000000,
    0xFFFF0000,
    0xFF00FF00,
    0xFFFFFF00,
    0xFF0000FF,
    0xFFFF00FF,
    0xFF00FFFF,
    0xFFFFFFFF
};

void ansi_handle_char(char c) {
    if (ansi.esc) {
        if (c == '[') {
            ansi.csi = 1;
            ansi.param_count = 0;
            memset(ansi.params, 0, sizeof(ansi.params));
        }
        ansi.esc = 0;
        return;
    }

    if (ansi.csi) {
        if (c >= '0' && c <= '9') {
            ansi.params[ansi.param_count] =
                ansi.params[ansi.param_count] * 10 + (c - '0');
            return;
        }

        if (c == ';') {
            ansi.param_count++;
            return;
        }

        if (c == 'm') {
            for (int i = 0; i <= ansi.param_count; i++) {
                int p = ansi.params[i];

                if (p == 0) {
                    ansi.fg = 0xFFFFFFFF;
                    ansi.bg = 0x00000000;
                }
                else if (p >= 30 && p <= 37) {
                    ansi.fg = ansi_basic_colours[p - 30];
                }
                else if (p == 38 && ansi.params[i + 1] == 2) {
                    uint8_t r = ansi.params[i + 2];
                    uint8_t g = ansi.params[i + 3];
                    uint8_t b = ansi.params[i + 4];
                    ansi.fg = 0x00000000 | (r << 16) | (g << 8) | b;
                    i += 4;
                }
                else if (p == 48 && ansi.params[i + 1] == 2) {
                    uint8_t r = ansi.params[i + 2];
                    uint8_t g = ansi.params[i + 3];
                    uint8_t b = ansi.params[i + 4];
                    ansi.bg = 0x00000000 | (r << 16) | (g << 8) | b;
                    i += 4;
                }
            }
        }

        ansi.csi = 0;
        return;
    }

    if (c == 0x1B) {
        ansi.esc = 1;
        return;
    }
    splatter_putc(get_tty_font(), c, ansi.fg, ansi.bg);
}