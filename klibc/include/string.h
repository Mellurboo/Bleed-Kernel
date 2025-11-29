#ifndef STRING_H
#define STRING_H 1

#include <stdint.h>

/// @brief gets the length of a string
/// @param string string to evaluate
/// @return uint32 length
uint32_t strlen(const char *string);

/// @brief compare two blocks of memory
/// @param s1 block 1
/// @param s2 block 2
/// @param n size to evaluate
/// @return result
int memcmp(const void *s1, const void *s2, uint64_t n);

/// @brief move memory from destination to source
/// @param dest destination
/// @param src source
/// @param n size to evaluate
/// @return void
void *memmove(void *dest, const void *src, uint64_t n);

/// @brief set memory to a location
/// @param s memory 
/// @param c address to set
/// @param n size
/// @return void
void *memset(void *s, int c, uint64_t n);

/// @brief copy memory from one loation to another
/// @param dest destination
/// @param src source
/// @param n size
/// @return void
void *memcpy(void *restrict dest, const void *restrict src, uint64_t n);

#endif