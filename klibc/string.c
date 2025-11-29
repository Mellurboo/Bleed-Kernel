#include <stdint.h>

/// @brief copy memory from one loation to another
/// @param dest destination
/// @param src source
/// @param n size
/// @return void
void *memcpy(void *restrict dest, const void *restrict src, uint64_t n) {
    uint8_t *restrict pdest = (uint8_t *restrict)dest;
    const uint8_t *restrict psrc = (const uint8_t *restrict)src;

    for (uint64_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

/// @brief set memory to a location
/// @param s memory 
/// @param c address to set
/// @param n size
/// @return void
void *memset(void *s, int c, uint64_t n) {
    uint8_t *p = (uint8_t *)s;

    for (uint64_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

/// @brief move memory from destination to source
/// @param dest destination
/// @param src source
/// @param n size to evaluate
/// @return void
void *memmove(void *dest, const void *src, uint64_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (uint64_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (uint64_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

/// @brief compare two blocks of memory
/// @param s1 block 1
/// @param s2 block 2
/// @param n size to evaluate
/// @return result
int memcmp(const void *s1, const void *s2, uint64_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (uint64_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

/// @brief gets the length of a string
/// @param string string to evaluate
/// @return uint32 length
uint32_t strlen(const char *string){
    uint32_t length = 0;
    while (*string != '\0'){
        length++;
        string++;
    }
    return length;
}
