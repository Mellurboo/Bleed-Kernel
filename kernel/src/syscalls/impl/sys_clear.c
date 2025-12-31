#include <devices/type/tty_device.h>
#include <stdio.h>

uint64_t sys_clear(uint64_t fd){
    device_t *dev = NULL;

    switch (fd) {
    case 1: case 2:
        dev = device_get_by_name("tty0");
        break;
    default:
        return (uint64_t)-1;
    }

    tty_t *tty0 = (tty_t *)dev->priv;
    tty0->ops->clear(tty0);
    return 0;
}