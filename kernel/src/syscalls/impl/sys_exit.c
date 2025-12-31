#include <threads/exit.h>
#include <stdio.h>
#include <syscalls/syscall.h>

void sys_exit(){
    exit();
}