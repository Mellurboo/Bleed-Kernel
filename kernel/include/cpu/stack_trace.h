#pragma once

#include <stdint.h>

/// @brief print the walk of RBP as hexidecimal
/// @param rbp frame pointer
void stack_trace_print(uint64_t *rbp);