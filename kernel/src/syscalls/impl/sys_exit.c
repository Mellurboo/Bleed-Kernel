#include <threads/exit.h>
#include <stdio.h>
#include "../syscall.h"

void sys_exit(){
    exit();
}