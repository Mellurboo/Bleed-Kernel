#include <stdint.h>
#include <stdio.h>
#include <sched/scheduler.h>
#include <ansii.h>
#include <threads/exit.h>

#include "syscall.h"

#define SYSCALL(idx, func) [idx] = (SyscallHandler)func

typedef uint64_t (*SyscallHandler)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

enum {
    SYS_READ,
    SYS_WRITE,
    SYS_OPEN,
    SYS_CLOSE,
    SYS_MKDIR,
    SYS_RMDIR,
    SYS_EXIT,
    SYS_PRINT
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
SyscallHandler syscall_handlers[] = {
    SYSCALL(SYS_EXIT, sys_exit),
    SYSCALL(SYS_PRINT, sys_print)
};
#pragma GCC diagnostic pop

uint64_t syscall_dispatch(cpu_context_t *cpu_ctx){
    return syscall_handlers[cpu_ctx->rax](cpu_ctx->rdi, cpu_ctx->rsi, cpu_ctx->rdx, cpu_ctx->r10, cpu_ctx->r8, cpu_ctx->r9);
}