#ifndef PMM_H
#define PMM_H

#include <stdint.h>

/// @brief gets the size of physical memory available
/// @return unsigned 64 memory size in bytes
uint64_t get_usable_pmem_size();

#endif