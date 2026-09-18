/*
 * Linux Platform Layer - Stub Implementation
 * For freestanding builds without full platform support
 */

#ifndef Platform_Linux_Kernel_h
#define Platform_Linux_Kernel_h

#include "../../../Pure.h"

/* Stub implementation for exit */
static inline void linux_exit(uint8_t exitcode)
{
    /* In a freestanding environment, we can't actually exit.
     * This is a stub that will be replaced when running on real Linux. */
    (void)exitcode;
    
    /* Infinite loop as fallback */
    while (1) { }
}

#endif /* Platform_Linux_Kernel_h */
