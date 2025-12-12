#include <stdint.h>
#include <drivers/pic/pic.h>

// busy looper? hardly know her

/// @brief wait for specified time in ms
/// @param millis ms value
void sleep_ms(uint32_t millis) {
    pit_countdown = millis / 10;
    while (pit_countdown > 0) {
        asm volatile("hlt");
    }
}
