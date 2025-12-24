#include <sched/scheduler.h>
#include <panic.h>
#include <mm/heap.h>
#include <string.h>
#include "priv_scheduler.h"
#include <mm/paging.h>

extern task_t *task_queue;
extern task_t *task_list_head;
static uint64_t next_pid = 1;

static void queue_task(task_t *task) {
    if (!task_queue) {
        task_queue = task;
        task->next = task;
        return;
    }

    task_t *tail = task_queue;
    while (tail->next != task_queue)
        tail = tail->next;

    tail->next = task;
    task->next = task_queue;
}

int sched_create_task(uint64_t cr3, uint64_t entry, uint64_t cs, uint64_t ss) {
    task_t *task = kmalloc(sizeof(task_t));
    if (!task) ke_panic("Failed to allocate task");

    task->id = next_pid++;
    task->state = TASK_READY;
    task->quantum_remaining = QUANTUM;

    task->page_map = cr3;

    task->kernel_stack = kmalloc(KERNEL_STACK_SIZE);
    if (!task->kernel_stack)
        ke_panic("Failed to allocate task stack");

    uint64_t top = (uint64_t)task->kernel_stack + KERNEL_STACK_SIZE;

    // Place CPU context at the top of stack
    cpu_context_t *ctx = (cpu_context_t *)(top - sizeof(cpu_context_t));
    memset(ctx, 0, sizeof(cpu_context_t));

    ctx->rip = entry;
    ctx->cs = cs;
    ctx->ss = ss;
    ctx->rflags = 0x202;
    ctx->rsp = top;

    task->context = ctx;

    queue_task(task);
    if (!task_list_head) task_list_head = task;

    return task->id;
}

void itterate_each_task(task_itteration_fn fn, void *userdata) {
    if (!task_list_head || !fn)
        return;

    task_t *task = task_list_head;
    do {
        fn(task, userdata);
        task = task->next;
    } while (task != task_list_head);
}

uint64_t get_task_count(void) {
    if (!task_list_head)
        return 0;

    uint64_t count = 0;
    task_t *task = task_list_head;

    do {
        count++;
        task = task->next;
    } while (task != task_list_head);

    return count;
}