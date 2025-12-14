//trying a different code style here, hope its nice ;p mainly using less oneliners cause they get hard to read after a while
//i felt the sched was a good place to do it cause i kind NEED this code to be easy to read
#include <stdint.h>
#include <stddef.h>
#include <sched/scheduler.h>
#include <status.h>
#include <stdio.h>
#include <ansii.h>

uint64_t task_count = 0;
task_t tasks[MAX_TASKS] = {0};

static task_t *current_task = NULL;
static task_t *task_queue = NULL;

static void queue_task(task_t *task){
    if (!task_queue) {
        task_queue = task;
        task->next = task;
        return;
    }

    task_t *tail = task_queue;
    while(tail->next != task_queue)
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

    return task;
}

/// @brief apply a task for the scheduler queue
/// @param entry entry point address
/// @return success
int scheduler_apply_task(void (*entry)(void)){
    if (task_count >= MAX_TASKS) return -SCHEDULER_OVERFLOW_ATTEMPT; // considering if i should panic here, we will see how it goes
    task_t *task = &tasks[task_count];
    task->id = task_count;
    task->state = TASK_READY;
    task->quantum_remaining = QUANTUM;

    uint64_t task_stack_top = (uint64_t)&task->kernel_stack[KERNEL_STACK_SIZE];
    cpu_context_t *context  = (cpu_context_t *)(task_stack_top - sizeof(cpu_context_t));

    // remove garbage
    for (size_t i = 0; i < sizeof(cpu_context_t) / sizeof(uint64_t); i++){
        ((uint64_t*)context)[i] = 0;
    }

    context->rip = (uint64_t)entry;
    context->cs  = 0x08;
    context->ss  = 0x10;
    context->rflags = 0x202;
    context->rsp = task_stack_top;

    task->context = context;

    queue_task(task);
    task_count++;

    return task->id;
}

void init_scheduler(void){
    asm volatile("cli");
    current_task = &tasks[0];
    current_task->state = TASK_RUNNING;
    current_task->quantum_remaining = QUANTUM;
    task_queue = current_task;
    asm volatile("sti");
}

cpu_context_t *scheduler_tick(cpu_context_t *context){

    current_task->context = context;

    if (--current_task->quantum_remaining > 0)
        return context;

    current_task->quantum_remaining = QUANTUM;
    current_task->state = TASK_READY;

    task_t *next = get_next_ready_task();
    next->state = TASK_RUNNING;

    current_task = next;

    return next->context;
}

void scheduler_init_bootstrap(void *rsp) {
    task_t *task = &tasks[0];

    task->id = 0;
    task->state = TASK_RUNNING;
    task->quantum_remaining = QUANTUM;
    task->context = (cpu_context_t *)rsp;

    task->next = task;
    current_task = task;
    task_queue = task;
    task_count = 1;
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

uint64_t get_task_count(){ return task_count; }
task_t get_task_from_tid(uint64_t tid) { return tasks[tid]; }