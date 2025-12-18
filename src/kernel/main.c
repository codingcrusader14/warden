#include <stdint.h>
#include <stddef.h>
#include "../drivers/qemu/pl011.h" 

char buffer[10];

void kernel_main(void) {
    pl011 serial;
    pl011_setup_qemu(&serial);

    
    send_message("Ready to receive...\n");
    char a = get_char();
    send_message("Received: ");
    put_char(a);
    put_char('\n');
}
