#define NANOPRINTF_IMPLEMENTATION
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS     0
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS       0
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS           1
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS           1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS          0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS       0
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS           0

#include <lib/flanterm/framebuffer.h>
#include <lib/flanterm/flanterm.h>
#include <string.h>
#include <stdarg.h>
#include <lib/nanoprintf.h>

/// @brief formatted print to tty
/// @param s string
void kprintf(const char *fmt, ...){
    char buf[1024];      // TODO make dynamic later
    
    va_list args;
    va_start(args, fmt);

    npf_vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    flanterm_write(get_flanterm_context(), buf, strlen(buf));
}