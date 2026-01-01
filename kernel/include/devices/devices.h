#pragma once

#include <stddef.h>
#include <stdint.h>

#define MAX_DEVICES 64  // change this up later for now im avoiding flexable array members

typedef struct device{
    const char *name;
    long (*read)(struct device *, void *buffer, size_t length);
    int (*write)(struct device *, const void *buffer, size_t length);
    int (*ioctl)(struct device *, unsigned long request, void *argument);
    void *priv;
} device_t;

int device_register(device_t *device);
device_t *device_get_by_name(const char *name);