#include <stdint.h>
#include "../drivers/qemu/uart.h" 

void kernel_main (void) {
    pl011 serial;
    pl011_setup_qemu(&serial);
    pl011_send_qemu(&serial, "Hello World\n");
} 
