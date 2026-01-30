#include <stdint.h>
#include <stddef.h>
#include "../drivers/qemu/pl011.h" 

#define LINE_SIZE 100

char buffer[LINE_SIZE];

void kernel_main(void) {
    pl011 serial;
    pl011_setup_qemu(&serial);

    char buffer[100];
    send_message("Ready to receive...\n");
    int c;
    int i = 0;
    while ((c = get_char()) != '\r') {
      buffer[i++] = c;
    }
    buffer[i] = '\0';
    send_message("Data obtained!");
    put_char('\n');
    send_message("I just entered\n");
    send_message(buffer);
  
}
