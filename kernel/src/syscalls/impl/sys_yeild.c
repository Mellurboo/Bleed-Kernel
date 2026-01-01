#include <sched/scheduler.h>
#include <stdio.h>

void sys_yield(){
    sched_yield();
}