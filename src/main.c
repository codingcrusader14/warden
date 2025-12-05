#include <stdint.h>
#include "memory_layout.h"

void kernel_main (void) {
    volatile uint64_t *const uart = (volatile uint64_t *) UART;
    const char *message = "Lets get this to repeat baby!!!";

        while (*message) {
            *uart = *message++;
        }
} 
