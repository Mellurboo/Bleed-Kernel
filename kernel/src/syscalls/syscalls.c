#include <stdint.h>
#include <stdio.h>
#include <sched/scheduler.h>
#define SYSCALL(idx, func) [idx] = (SyscallHandler)func

typedef uint64_t (*SyscallHandler)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

enum {
    SYS_HELLO_WORLD
};

void sys_hello_world(){
    kprintf("Hello from userspace!\n");
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
SyscallHandler syscall_handlers[] = {
    SYSCALL(SYS_HELLO_WORLD, sys_hello_world),
};
#pragma GCC diagnostic pop

uint64_t syscall_dispatch(cpu_context_t *cpu_ctx){
    return syscall_handlers[cpu_ctx->rax](cpu_ctx->rdi, cpu_ctx->rsi, cpu_ctx->rdx, cpu_ctx->r10, cpu_ctx->r8, cpu_ctx->r9);
}