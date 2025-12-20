#define NANOPRINTF_IMPLEMENTATION
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS     0
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS       0
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS           1
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS           1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS          0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS       0
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS           0

#include <drivers/framebuffer/framebuffer.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <lib/nanoprintf.h>
#include <mm/heap.h>

/// @brief formatted print to tty
/// @param s string
void kprintf(const char *fmt, ...){
    size_t size = 256;
    char *buf = NULL;

    while (1) {
        buf = kmalloc(size);
        if (!buf) return;

        va_list args;
        va_start(args, fmt);
        int written = npf_vsnprintf(buf, size, fmt, args);
        va_end(args);

        if (written < 0) {
            kfree(buf, size);
            return;
        }

        if ((size_t)written < size) {
            break;
        }
        kfree(buf, size);
        size = size * 2;
    }

    framebuffer_write_string(buf);
    kfree(buf, size);
}
