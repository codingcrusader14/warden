#include "../../drivers/qemu/uart.h"

static volatile uint32 *access_register(const struct pl011 *dev, uint16 offset) {
    const uint64 address = dev->base_address + offset;
    return (volatile uint32*) ((void*) address);
}

static void calculate_divsors(const struct pl011 *dev, uint32 *integer, uint32 *fractional) {
    const uint32 divisor = (64 * dev->clock) / (16 * dev->baud_rate);

    *fractional = divisor & 0x3F; // lower 16 for UARTIBRD
    *integer = (divisor >> 6) & 0xFFFF; // lower 6 for UARTFBRD
}

static void check_transmission(const struct pl011 *dev) {
    while ((*access_register(dev, FLAG_OFFSET)) & (FLAG_BUSY)) {}
}

static int pl011_reset_qemu(const pl011 *dev) {
    const uint32 control_register = *access_register(dev, CONTROL_OFFSET);
    uint32 line_control_register = *access_register(dev, LINE_CONTROL_OFFSET);
    uint32 ibrd, fbrd;

    /* Disable UART */
    *access_register(dev, CONTROL_OFFSET) = control_register & CR_UARTEN;

    /* Wait for end of transmission */
    check_transmission(dev);

    /* Flush transmit FIFO */
    *access_register(dev, LINE_CONTROL_OFFSET) = (line_control_register & ~LCR_FEN);

    /* Set frequency divisor (UARTIBRD and UARTFBRD) to configure speed*/
    calculate_divsors(dev, &ibrd, &fbrd);
    *access_register(dev, INTEGER_BAUD_RATE_OFFSET) = ibrd;
    *access_register(dev, FRACTIONAL_BAUD_RATE_OFFSET) = fbrd;

    /* Reset line control */
    *access_register(dev, LINE_CONTROL_OFFSET) = 0x0;

    uint32 bit_field = (dev->data_bits - 5); // maps 5 -> 0, 6 -> 1, 7 -> 2, 8 -> 3
    *access_register(dev, LINE_CONTROL_OFFSET) |= (bit_field & 0x3) << 5;
    
    if (dev->stop_bits == 2)
        *access_register(dev, LINE_CONTROL_OFFSET) |= LCR_STP2;

    *access_register(dev, LINE_CONTROL_OFFSET) |= LCR_FEN;

    /* Reset DMA */
    *access_register(dev, DMA_CONTROL_OFFSET) = 0x0;

    /* Enable UART transmission/receiving */
    *access_register(dev, CONTROL_OFFSET) = (CR_TXEN | CR_UARTEN | CR_EXN);
    
    return 0;
}

int pl011_setup_qemu(pl011 *dev) {
    dev->base_address = QEMU_PL011_BASE;
    dev->clock = QEMU_CLOCK;
    dev->baud_rate = QEMU_BAUD_RATE;
    dev->data_bits = QEMU_DATA_BITS;
    dev->stop_bits = QEMU_STOP_BITS;
    return pl011_reset_qemu(dev);
}

int pl011_send_qemu(pl011 *dev, const char *data) {
    check_transmission(dev);
    while (*data) {
        *access_register(dev, DATA_OFFSET) = *data++;
        check_transmission(dev);
    }
    return 0;
}

