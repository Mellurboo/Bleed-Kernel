#ifndef PANIC_H
#define PANIC_H

/// @brief Raise an exception and halt the system
/// @param reason the kernel can choose a reason to relay to the user
void ke_panic(const char* reason);

#endif