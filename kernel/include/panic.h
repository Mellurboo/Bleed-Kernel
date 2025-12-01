#ifndef KPANIC_H
#define KPANIC_H

/// @brief prints cpu register info and stops the system
/// @param reason reason for the panic the system will provide this if its automatic
void kpanic(const char* reason);

#endif