#include <stdio.h>
#include "../syscall.h"

void sys_print(const char *message){
    kprintf("%s", message);
}