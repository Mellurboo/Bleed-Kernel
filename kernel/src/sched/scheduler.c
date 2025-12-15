#include <sched/scheduler.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <panic.h>
#include "priv_scheduler.h"
#include <mm/paging.h>
#include <stdio.h>

task_t *current_task   = NULL;
task_t *task_queue     = NULL;
task_t *task_list_head = NULL;

task_t *dead_task_head = NULL;
task_t *dead_task_tail = NULL;

task_t *get_current_task() {
    return current_task;
}

void init_scheduler(void) {
    asm volatile ("cli");
    task_queue = task_list_head;
    asm volatile ("sti");
}

static inline void switch_to_task(task_t *task) {
    if (!task || !task->cr3)
        return;

    write_cr3(task->cr3);
}

cpu_context_t *sched_tick(cpu_context_t *context) {
    if (!current_task)
        return context;
    current_task->context = context;

    if (current_task->quantum_remaining > 0)
        current_task->quantum_remaining--;

    if (current_task->quantum_remaining > 0)
        return context;

    
    current_task->quantum_remaining = QUANTUM;

    if (current_task->state == TASK_RUNNING)
        current_task->state = TASK_READY;

    task_t *task = current_task->next;
    task_t *start = task;

    do {
        if (task->state == TASK_READY) {
            current_task = task;
            current_task->state = TASK_RUNNING;
            switch_to_task(current_task);
            return current_task->context;
        }
        task = task->next;
    } while (task != start);

    current_task->state = TASK_RUNNING;
    kprintf("Tick: current=%llu, quantum=%u, next=%llu\n",
        current_task->id, current_task->quantum_remaining, current_task->next->id);
    switch_to_task(current_task);

    return current_task->context;
}

void sched_bootstrap(void *rsp) {
    task_t *kernel_task = kmalloc(sizeof(task_t));
    if (!kernel_task)
        ke_panic("Failed to allocate kernel task");

    kernel_task->id                = 0;
    kernel_task->state             = TASK_RUNNING;
    kernel_task->quantum_remaining = QUANTUM;
    kernel_task->context           = (cpu_context_t *)rsp;
    kernel_task->next              = kernel_task;

    kernel_task->cr3 = read_cr3();

    current_task   = kernel_task;
    task_queue     = kernel_task;
    task_list_head = kernel_task;
}

void sched_yield(void) {
    asm volatile ("cli");

    if (current_task &&
        current_task->state == TASK_RUNNING) {

        current_task->quantum_remaining = 0;
        current_task->state = TASK_READY;
    }

    asm volatile ("sti");
    asm volatile ("int $32");
}