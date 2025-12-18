/*
 * UART driver for Armv8-A (QEMU virt)
 * 
 * Provides serial output and input to the console
 * Hardware : Arm PrimeCell UART PL011
 */
 
#include "../../kernel/types.h"

/* Qemu virt mode */
#define QEMU_PL011_BASE 0x9000000UL
#define QEMU_CLOCK 24000000 // 24 * 10^6 = Hz
#define QEMU_BAUD_RATE 38400
#define QEMU_DATA_BITS 8
#define QEMU_STOP_BITS 1

/* PL011 Registers */
enum pl011_registers {
  DATA_OFFSET                        = 0x000,
  FLAG_OFFSET                        = 0x018, // RO
  IRDA_LOW_POWER_COUNTER_OFFSET      = 0x020,
  INTEGER_BAUD_RATE_OFFSET           = 0x024,
  FRACTIONAL_BAUD_RATE_OFFSET        = 0x028,
  LINE_CONTROL_OFFSET                = 0x02C,
  CONTROL_OFFSET                     = 0x030,
  INTERRUPT_FIFO_LEVEL_SELECT_OFFSET = 0x034,
  INTERRUPT_MASK_SET_CLEAR_OFFSET    = 0x038,
  RAW_INTERRUPT_STATUS_OFFSET        = 0x03C, // RO
  MASKED_INTERRUPT_STATUS_OFFSET     = 0x040, // RO
  INTERRUPT_CLEAR_OFFSET             = 0x044, // WO
  DMA_CONTROL_OFFSET                 = 0x048,
};

/* Register specific bits */
enum flag {
  FLAG_RXFF = (1 << 6), // Recieve FIFO full
  FLAG_RXFE = (1 << 4), // Recieve FIFO empty
  FLAG_BUSY = (1 << 3),
};

enum control {
    CR_RXE = (1 << 9),
    CR_TXE = (1 << 8),
    CR_UARTEN = (1 << 0),
};

enum line_control {
    LCR_FEN = (1 << 4),
    LCR_STP2 = (1 << 3),   
};

typedef struct pl011 {
  uint64 clock;
  uint32 baud_rate;
  uint32 data_bits;
  uint32 stop_bits;
} pl011;

int pl011_setup_qemu(pl011 *dev);
int send_message(const char *data);
int get_char();
void put_char(char c);





