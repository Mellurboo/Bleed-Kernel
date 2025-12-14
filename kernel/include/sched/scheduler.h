#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stddef.h>

#define KERNEL_STACK_SIZE   8196
#define MAX_TASKS           64
#define QUANTUM             5

typedef struct cpu_context {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t vector;
    uint64_t error;
    uint64_t rip, cs, rflags, rsp, ss;
} cpu_context_t;

typedef enum {
    TASK_FREE,
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_DEAD
} task_state_t;

typedef struct task {
    uint64_t        id;
    task_state_t    state;
    cpu_context_t  *context;

    uint8_t kernel_stack[KERNEL_STACK_SIZE];
    uint32_t quantum_remaining;

    struct task *next;
} task_t;

int scheduler_apply_task(void (*entry)(void));
extern void scheduler_init_bootstrap(void *rsp);
extern cpu_context_t *scheduler_tick(cpu_context_t *context);

void task_exit(void);
const char *task_state_str(task_state_t state);
task_t get_task_from_tid(uint64_t tid);
uint64_t get_task_count();
#endif