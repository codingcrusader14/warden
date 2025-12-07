#include "../../drivers/qemu/uart.h"

void uartputc(char c) {
    while ((UART_FLAG_REGISTER  >> 5) & 1) {
        ;
    }
    UART_DATA_REGISTER = c;
}
