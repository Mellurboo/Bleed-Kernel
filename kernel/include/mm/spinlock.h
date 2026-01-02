#pragma once
#include <stdint.h>

typedef struct {
    volatile uint32_t lock;
} spinlock_t;

static inline void spinlock_init(spinlock_t *lock) {
    lock->lock = 0;
}

static inline void spinlock_acquire(spinlock_t *lock) {
    while (__atomic_test_and_set(&lock->lock, __ATOMIC_ACQUIRE)) {
        __asm__ volatile("pause");
    }
}

static inline void spinlock_release(spinlock_t *lock) {
    __atomic_clear(&lock->lock, __ATOMIC_RELEASE);
}
