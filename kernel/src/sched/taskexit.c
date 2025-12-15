#include <sched/scheduler.h>
#include <panic.h>
#include <drivers/serial/serial.h>
#include <ansii.h>

__attribute__((noreturn))
void exit(void) {
    task_t *current_task = get_current_task();

    if (current_task->id == 0 || current_task->id == 1)
        ke_panic("Critical Thread Died");

    serial_printf(
        "%sTask %d has exited, marking as dead for reaping\n",
        LOG_INFO,
        current_task->id
    );

    sched_mark_task_dead();
    sched_yield();

    for (;;) { asm volatile ("hlt"); }
}