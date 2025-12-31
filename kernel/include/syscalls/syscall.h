#pragma once

#include <stdio.h>
#include <stdint.h>

void sys_exit();
uint64_t sys_write(uint64_t fd, uint64_t buf, uint64_t len);
uint64_t sys_clear(uint64_t fd);