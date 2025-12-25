#include <threads/exit.h>
#include "../syscall.h"

void sys_exit(){
    exit();
}