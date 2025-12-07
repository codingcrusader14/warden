#include <stdint.h>
#include "../drivers/qemu/uart.h" 

void kernel_main (void) {
    const char *message = "MERRY CHRISTMAS";
    while (*message) {
        uartputc(*message++);
    }
} 
