#include <drivers/ps2/ps2_keyboard.h>
#include <drivers/pic/pic.h>
#include <cpu/io.h>
#include <stddef.h>
#include <stdbool.h>

#define KBD_PORT 0x60

static bool shift = false;
static bool caps = false;

#define BUF_SIZE 128
static volatile char buf[BUF_SIZE];
static volatile int head = 0, tail = 0;

static keyboard_callback_t kb_callback = NULL;

/// @brief push a char onto the keyboard buffer stack
static void push_char(char c) {
    int next = (head + 1) % BUF_SIZE;
    if (next != tail) {
        buf[head] = c;
        head = next;
    }
}

/// @brief pop a char off the keyboard buffer stack 
static int pop_char(void) {
    if (tail == head) return -1;
    char c = buf[tail];
    tail = (tail + 1) % BUF_SIZE;
    return c;
}

/// @brief keyboard request from pic
/// @param irq value
void ps2_keyboard_irq(uint8_t irq) {
    uint8_t sc = inb(KBD_PORT);

    if (sc & 0x80) {
        sc &= ~0x80;
        if (sc == 0x2A || sc == 0x36) shift = false;
        pic_eoi(irq);
        return;
    }

    if (sc == 0x2A || sc == 0x36) { shift = true; pic_eoi(irq); return; }
    if (sc == 0x3A) { caps = !caps; pic_eoi(irq); return; }

    char c = 0;
    if (sc < 128) {
        c = shift ? keymap_shift[sc] : keymap[sc];
        if (c >= 'a' && c <= 'z' && (caps ^ shift)) c -= 32;
    }

    if (c && kb_callback) {
        kb_callback(c);
    }

    if (c) push_char(c);
    pic_eoi(irq);
}

/// @brief blocking get char
/// @return char ascii code int
int keyboard_get_char(void) {
    int c;
    while ((c = pop_char()) == -1)
        __asm__ volatile("hlt");
    return c;
}

void keyboard_set_callback(keyboard_callback_t cb) {
    kb_callback = cb;
}

