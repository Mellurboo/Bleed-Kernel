#include <string.h>
#include <devices/devices.h>
#include <mm/heap.h>
#include <devices/type/tty_device.h>

uint64_t sys_read(uint64_t fd, uint64_t user_buf, uint64_t len){
    if (!user_buf || len == 0)
        return 0;

    device_t *dev = NULL;
    if (fd == 1) dev = device_get_by_name("tty0");
    if (!dev || !dev->read) return -1;

    return dev->read(dev, (void *)user_buf, len);
}