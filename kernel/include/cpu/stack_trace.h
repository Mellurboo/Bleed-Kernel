#ifndef STACK_TRACE_H
#define STACK_TRACE_H

#include <stdint.h>

void print_stack_trace(uint64_t *rbp);

#endif