#include "../../drivers/qemu/uart.h"

void uart_init() {
    check_transmission();
    disable_uart();
    enable_uart();
}

void uart_put_c(char c) {
    while (UART_TX_FIFO_FULL) {
        ;
    }
    UART_DATA_REGISTER = c;
}

int32 uart_get_c() {
    while (UART_RX_FIFO_EMPTY) {
        ;
    }
    return UART_DATA_REGISTER; 
}
