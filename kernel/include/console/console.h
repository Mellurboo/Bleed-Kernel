#pragma once
#include <devices/devices.h>

void console_set(device_t *console_device);
device_t *console_get_active_console(void);

int console_write(const void *string);