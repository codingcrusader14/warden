#include <stdint.h>
#include "../drivers/qemu/uart.h" 

void kernel_main (void) {
    uart_init();
    const char *message = "MERRY CHRISTMAS";
    while (*message) {
        uart_put_c(*message++);
    }

    int length = 10;
    char word[length];
    for (int i = 0; i < length - 1; ++i) {
        word[i] = uart_get_c();
    }

    word[length - 1] = '\0';
    for (int i = 0; i < length - 1; ++i) {
        uart_put_c(word[i]);
    }

} 
