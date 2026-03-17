#include "pl011.h"
#include "../../kernel/libk/includes/stdio.h"

static pl011 uart;
static uart_buf ubuf;

static inline uint32 pl011_read(enum pl011_registers offset) {
    return *(volatile uint32*) (QEMU_PL011_BASE + offset);
}

static inline void pl011_write(enum pl011_registers offset, uint32 value) {
    *(volatile uint32*) (QEMU_PL011_BASE + offset) = value;
}

static void calculate_divsors(const pl011 *dev, uint32 *integer, uint32 *fractional) {
    const uint32 divisor = (64 * dev->clock) / (16 * dev->baud_rate);

    *fractional = divisor & 0x3F; // lower 6 for UARTFBRD
    *integer = (divisor >> 6) & 0xFFFF; // lower 16 for UARTIBRD
}

static void uart_flush() {
    while ((pl011_read(FLAG_OFFSET)) & (FLAG_BUSY)) {}
}

static void wait_tx_ready() {
  while (pl011_read(FLAG_OFFSET) & FLAG_TXFF) {}
}

static void wait_rx_ready() {
  while (pl011_read(FLAG_OFFSET) & FLAG_RXFE) {}
}

static void uart_buf_init() {
  ubuf.read = 0;
  ubuf.write = 0;
  lock_init(&ubuf.spinlock);
  wait_queue_init(&ubuf.rx_wait);
}


static int pl011_reset_qemu(const pl011 *dev) {
    const uint32 control_register = pl011_read(CONTROL_OFFSET);
    uint32 line_control_register = pl011_read(LINE_CONTROL_OFFSET);
    uint32 ibrd, fbrd;

    /* Wait for end of transmission */
    uart_flush();

    /* Disable UART */
    pl011_write(CONTROL_OFFSET, control_register & ~(CR_UARTEN | CR_TXE | CR_RXE));

    /* Flush transmit FIFO */
    pl011_write(LINE_CONTROL_OFFSET, (line_control_register & ~LCR_FEN));   

    
    /* Set frequency divisor (UARTIBRD and UARTFBRD) to configure speed*/
    calculate_divsors(dev, &ibrd, &fbrd);
    pl011_write(INTEGER_BAUD_RATE_OFFSET, ibrd);
    pl011_write(FRACTIONAL_BAUD_RATE_OFFSET, fbrd);

    /* Reset line control
     * Word Length : 8 bits
     * Enable FIFO
     * Select stop bits : 1 bit
     * No parity enabled
     */
    line_control_register = 0;

    uint32 bit_field = (dev->data_bits - 5); // maps 5 -> 0, 6 -> 1, 7 -> 2, 8 -> 3
    line_control_register |= (bit_field & 0x3) << 5;
    line_control_register |= LCR_FEN;
    if (dev->stop_bits == 2)
        line_control_register |= LCR_STP2;

    pl011_write(LINE_CONTROL_OFFSET, line_control_register);

    /* Clear Interrupts */
    pl011_write(INTERRUPT_CLEAR_OFFSET, 0x7FF); // bits 11:15 are reserved

    /* Mask (disable) all interrupts */
    pl011_write(INTERRUPT_MASK_SET_CLEAR_OFFSET, (0x000 | IMSC_RXIM)); // bits 11:15 are reserved
  
    
    /* Disable DMA */
    uint32 dma_register = pl011_read(DMA_CONTROL_OFFSET);
    pl011_write(DMA_CONTROL_OFFSET, (dma_register & ~(0x7))); // bits 3:15 are reserved

    /* Enable UART transmission/receiving */
    pl011_write(CONTROL_OFFSET, (CR_TXE | CR_UARTEN | CR_RXE));

    /* Initialize UART buffer */
    uart_buf_init();
    return 0;
}


static int pl011_setup_qemu(pl011 *dev) {
    dev->clock = QEMU_CLOCK;
    dev->baud_rate = QEMU_BAUD_RATE;
    dev->data_bits = QEMU_DATA_BITS;
    dev->stop_bits = QEMU_STOP_BITS;
    return pl011_reset_qemu(dev);
}

void uart_init() {
  pl011_setup_qemu(&uart);
}

int send_message(const char *data) {
    while (*data) {
      wait_tx_ready();
      pl011_write(DATA_OFFSET, *data++);
    }
    return 0;
}

int get_char() {
    wait_rx_ready();
    int c = pl011_read(DATA_OFFSET);
    return c;
}

void put_char(char c) {
    wait_tx_ready();
    pl011_write(DATA_OFFSET, c);
}

static void put(char value) {
  if (ubuf.write - ubuf.read == UART_BUF_SIZE) return; // buffer full drop data

  ubuf.buffer[ubuf.write & (UART_BUF_SIZE - 1)] = value;
  ubuf.write++;
}

static char get() {
  char tmp = ubuf.buffer[ubuf.read & (UART_BUF_SIZE - 1)];
  ubuf.read++;
  return tmp;
}

void uart_isr() {
  lock(&ubuf.spinlock); 
  while (!(pl011_read(FLAG_OFFSET) & FLAG_RXFE)) {
    char c = pl011_read(DATA_OFFSET);
    put(c);
  }
  wait_queue_wakeup(&ubuf.rx_wait);
  pl011_write(INTERRUPT_CLEAR_OFFSET, ICR_RXIC);
  unlock(&ubuf.spinlock);
}

int uart_read() {
  lock(&ubuf.spinlock);

  while (ubuf.read == ubuf.write) {
    wait_queue_sleep(&ubuf.rx_wait, &ubuf.spinlock);
  }

  int c = get();
  unlock(&ubuf.spinlock);
  return c;
}

int uart_try_read() {
  lock(&ubuf.spinlock);
  if (ubuf.read == ubuf.write) {
    unlock(&ubuf.spinlock);
    return -1;
  }
  int c = get();
  unlock(&ubuf.spinlock);
  return c;
}


 
