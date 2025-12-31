#include <devices/type/tty_device.h>
#include <sched/scheduler.h>

int tty_read(device_t *dev, void *buf, size_t len){
    tty_t *tty = dev->priv;
    size_t i = 0;
    
    while (i < len) {
        while (tty->in_head == tty->in_tail) {
            asm volatile("hlt");
        }

        char c = tty->inbuffer[tty->in_tail++ % TTY_BUFFER_SZ];
        ((char*)buf)[i++] = c;

        if ((tty->flags & TTY_CANNONICAL) && c == '\n')
            break;
    }

    return i;
}

int tty_write(device_t *dev, const void *buf, size_t len) {
    tty_t *tty = dev->priv;
    const char *c = buf;

    for (size_t i = 0; i < len; i++)
        tty->ops->putchar(tty, c[i]);
    return len;
}

int tty_ioctl(device_t *dev, unsigned long req, void *arg){
    tty_t *tty = dev->priv;

    switch (req) {
    case 0:
        *(uint32_t *)arg = tty->flags;
        return 0;

    case 1:
        tty->flags = *(uint32_t *)arg;
        return 0;
    }

    return -1;
}
