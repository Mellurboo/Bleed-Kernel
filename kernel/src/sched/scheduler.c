//trying a different code style here, hope its nice ;p mainly using less oneliners cause they get hard to read after a while
//i felt the sched was a good place to do it cause i kind NEED this code to be easy to read
#include <stdint.h>
#include <stddef.h>
#include <sched/scheduler.h>
#include <drivers/serial/serial.h>
#include <status.h>
#include <stdio.h>
#include <ansii.h>
#include <panic.h>
#include <mm/heap.h>

#define MAX_REAPES_PER_CALL 16

static task_t *current_task = NULL;
static task_t *task_queue = NULL;
static task_t *task_list_head = NULL;
static uint64_t next_pid = 1; // TID 0 reserved for kernel
                              // TID 1 reserved for reaper (scheduled at bootstrap)

void scheduler_reap(void);

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

static task_t *get_next_ready_task(void){
    task_t *task = current_task->next;

    while(task != current_task){
        if (task->state == TASK_READY)
            return task;
        task = task->next;
    }

    if (current_task->state == TASK_RUNNING || current_task->state == TASK_READY)
        return current_task;

    return task_list_head;
}

/// @brief apply a task for the scheduler queue
/// @param entry entry point address
/// @return success
int sched_create_task(void (*entry)(void)) {
    task_t *task = kmalloc(sizeof(task_t));
    if (!task) ke_panic("Failed to allocate task");

    task->id = next_pid++;
    task->state = TASK_READY;
    task->quantum_remaining = QUANTUM;
    task->next = NULL;

    // allocate stack
    task->kernel_stack = kmalloc(KERNEL_STACK_SIZE);
    if (!task->kernel_stack) ke_panic("Failed to allocate task stack");
    uint64_t task_stack_top = (uint64_t)task->kernel_stack + KERNEL_STACK_SIZE;

    // setup ctx
    cpu_context_t *context = (cpu_context_t *)(task_stack_top - sizeof(cpu_context_t));
    memset(context, 0, sizeof(cpu_context_t));
    context->rip = (uint64_t)entry;
    context->cs = 0x08;
    context->ss = 0x10;
    context->rflags = 0x202;
    context->rsp = task_stack_top;
    task->context = context;

    queue_task(task);

    // update head ptr
    if (!task_list_head) task_list_head = task;

    return task->id;
}

void init_scheduler(void){
    asm volatile("cli");

    // Kernel task already created in bootstrap
    // current_task = task_list_head;
    task_queue = task_list_head;

    asm volatile("sti");
}

cpu_context_t *sched_tick(cpu_context_t *context){

    current_task->context = context;

    if (--current_task->quantum_remaining > 0)
        return context;

    current_task->quantum_remaining = QUANTUM;
    if (current_task->state == TASK_RUNNING)
        current_task->state = TASK_READY;

    task_t *next = get_next_ready_task();
    next->state = TASK_RUNNING;
    current_task = next;

    return next->context;
}

void sched_bootstrap(void *rsp) {
    task_t *kernel_task = kmalloc(sizeof(task_t));
    if (!kernel_task) ke_panic("Failed to allocate kernel task");

    kernel_task->id = 0;
    kernel_task->state = TASK_RUNNING;
    kernel_task->quantum_remaining = QUANTUM;
    kernel_task->context = (cpu_context_t *)rsp;
    kernel_task->next = kernel_task;

    current_task = kernel_task;
    task_queue = kernel_task;
    task_list_head = kernel_task;
}

__attribute__((noreturn))
void exit(void){
    if (current_task->id == 0 || current_task->id == 1)
        ke_panic("Critical Thread Died");

    serial_printf("%sTask %d has exited, marking as dead for reaping\n", LOG_INFO, current_task->id);
    current_task->state = TASK_DEAD;

    // Stop executing this task
    for (;;) { asm volatile("hlt"); }
}

void scheduler_reap(void) {
    if (!task_queue) return;

    task_t *prev = task_queue;
    task_t *cur = task_queue->next;

    int reap_count = 0;

    do {
        if (cur->state == TASK_DEAD && cur != current_task) {
            serial_printf("%sReaping Task %d\n", LOG_INFO, cur->id);
            reap_count++;
            prev->next = cur->next;

            kfree(cur->kernel_stack, KERNEL_STACK_SIZE);
            kfree(cur, sizeof(task_t));

            cur = prev->next;
            if (cur == task_queue)
                task_queue = cur->next;

            continue;
        }

        prev = cur;
        cur = cur->next;
    } while (cur && cur != task_queue && reap_count < MAX_REAPES_PER_CALL);

    for (;;) { asm volatile("hlt"); }
    exit();
}

// api stuff

const char *task_state_str(task_state_t state) {
    switch (state) {
        case TASK_READY:   return "\x1b[38;2;0;255;255mREADY\x1b[255;255;255;255;0m";
        case TASK_RUNNING: return "\x1b[38;2;0;255;0mRUNNING\x1b[255;255;255;255;0m";
        case TASK_BLOCKED: return "\x1b[38;2;128;128;128mBLOCKED\x1b[255;255;255;255;0m";
        case TASK_DEAD:    return "\x1b[38;2;255;0;0mDEAD\x1b[255;255;255;255;0m";
        default:           return "UNKNOWN";
    }
}

uint64_t get_task_count(void) {
    if (!task_list_head) return 0;

    uint64_t count = 0;
    task_t *task = task_list_head;

    do {
        count++;
        task = task->next;
    } while (task != task_list_head);

    return count;
}

void itterate_each_task(task_itteration_fn fn, void *userdata) {
    if (!task_list_head || !fn) return;

    task_t *task = task_list_head;
    do {
        fn(task, userdata);
        task = task->next;
    } while (task != task_list_head);
}
