#include "../../drivers/qemu/uart.h"

static inline uint32 pl011_read(enum pl011_registers offset) {
    return *(volatile uint32*) (QEMU_PL011_BASE + offset );
}

static inline void pl011_write(enum pl011_registers offset, uint32 value) {
    *(volatile uint32*) (QEMU_PL011_BASE + offset) = value;
}

static void calculate_divsors(const pl011 *dev, uint32 *integer, uint32 *fractional) {
    const uint32 divisor = (64 * dev->clock) / (16 * dev->baud_rate);

    *fractional = divisor & 0x3F; // lower 6 for UARTFBRD
    *integer = (divisor >> 6) & 0xFFFF; // lower 16 for UARTIBRD
}

static void check_transmission() {
    while ((pl011_read(FLAG_OFFSET)) & (FLAG_BUSY)) {}
}

static int pl011_reset_qemu(const pl011 *dev) {
    const uint32 control_register = pl011_read(CONTROL_OFFSET);
    uint32 line_control_register = pl011_read(LINE_CONTROL_OFFSET);
    uint32 ibrd, fbrd;

    /* Disable UART */
    pl011_write(CONTROL_OFFSET, control_register & ~(CR_UARTEN | CR_TXE | CR_RXE));

    /* Wait for end of transmission */
    check_transmission();

    /* Flush transmit FIFO */
    pl011_write(LINE_CONTROL_OFFSET, (line_control_register & ~LCR_FEN));

    /* Set frequency divisor (UARTIBRD and UARTFBRD) to configure speed*/
    calculate_divsors(dev, &ibrd, &fbrd);
    pl011_write(INTEGER_BAUD_RATE_OFFSET, ibrd);
    pl011_write(FRACTIONAL_BAUD_RATE_OFFSET, fbrd);

    /* Reset line control */
    line_control_register = 0;

    uint32 bit_field = (dev->data_bits - 5); // maps 5 -> 0, 6 -> 1, 7 -> 2, 8 -> 3
    line_control_register |= (bit_field & 0x3) << 5;
    
    if (dev->stop_bits == 2)
        line_control_register |= LCR_STP2;

    line_control_register |= LCR_FEN;
    pl011_write(LINE_CONTROL_OFFSET, line_control_register);

    /* Clear Interrupts */
    pl011_write(INTERRUPT_CLEAR_OFFSET, 0x7FF); // bits 11:15 are reserved

    /* Mask (disable) all interrupts */
    pl011_write(INTERRUPT_MASK_SET_CLEAR_OFFSET, 0x000); // bits 11:15 are reserved
    
    /* Reset DMA */
    pl011_write(DMA_CONTROL_OFFSET, 0x0);

    /* Enable UART transmission/receiving */
    pl011_write(CONTROL_OFFSET, (CR_TXE | CR_UARTEN | CR_RXE));
    
    return 0;
}

int pl011_setup_qemu(pl011 *dev) {
    dev->clock = QEMU_CLOCK;
    dev->baud_rate = QEMU_BAUD_RATE;
    dev->data_bits = QEMU_DATA_BITS;
    dev->stop_bits = QEMU_STOP_BITS;
    return pl011_reset_qemu(dev);
}

int pl011_send_qemu(const char *data) {
    check_transmission();
    while (*data) {
        pl011_write(DATA_OFFSET, *data++);
        check_transmission();
    }
    return 0;
}

int pl011_get_char_qemu() {
    while (pl011_read(FLAG_OFFSET) & (FLAG_RXFE)) {}
    return pl011_read(DATA_OFFSET);
}

void pl011_put_char_qemu(char c) {
    check_transmission();
    pl011_write(DATA_OFFSET, c);
}



