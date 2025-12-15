#include <stdint.h>
#include <stddef.h>
#include "../drivers/qemu/uart.h" 

void kernel_main (void) {
    pl011 serial;
    pl011_setup_qemu(&serial);
    char a;
    pl011_send_qemu("Please enter a character: ");
    a = pl011_get_char_qemu();
    pl011_put_char_qemu(a);
    
}
