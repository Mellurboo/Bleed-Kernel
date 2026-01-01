#pragma once

#include <devices/devices.h>

int tty_ioctl(device_t *dev, unsigned long req, void *arg);
int tty_write(device_t *dev, const void *buf, size_t len);
long tty_read(device_t *dev, void *buf, size_t len);
