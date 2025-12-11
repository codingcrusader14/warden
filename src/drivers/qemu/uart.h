/*
 * UART driver for Armv8-A (QEMU virt)
 * 
 * Provides serial output and input to the console
 * Hardware : Arm PrimeCell UART PL011
 */
 
#include "../../kernel/types.h"

/* Qemu virt mode */
#define QEMU_PL011_BASE 0x9000000;
#define QEMU_CLOCK 24000000 // 24 * 10^6 = Hz
#define QEMU_BAUD_RATE 38400
#define QEMU_DATA_BITS 8
#define QEMU_STOP_BITS 1

constexpr uint32 DATA_OFFSET = 0x000;
static constexpr uint32 RECIEVE_STATUS_OFFSET = 0x004;
static constexpr uint32 ERROR_OFFSET = 0x004;
static constexpr uint32 FLAG_OFFSET = 0x018;
static constexpr uint32 IRDA_LOW_POWER_COUNTER_OFFSET = 0x020;
static constexpr uint32 INTEGER_BAUD_RATE_OFFSET = 0x024;
static constexpr uint32 FRACTIONAL_BAUD_RATE_OFFSET = 0x028;
static constexpr uint32 LINE_CONTROL_OFFSET = 0x02C;
static constexpr uint32 CONTROL_OFFSET = 0x030;
static constexpr uint32 INTERRUPT_FIFO_LEVEL_SELECT_OFFSET = 0x034;
static constexpr uint32 INTERRUPT_MASK_SET_CLEAR_OFFSET = 0x038;
static constexpr uint32 RAW_INTERRUPT_STATUS_OFFSET = 0x03C;
static constexpr uint32 MASKED_INTERRUPT_STATUS_OFFSET = 0x040;
static constexpr uint32 INTERRUPT_CLEAR_OFFSET = 0x044;
static constexpr uint32 DMA_CONTROL_OFFSET = 0x048;

/* Register specific bits */
enum flag {
  FLAG_BUSY = (1 << 3),
};

enum control {
    CR_EXN = (1 << 9),
    CR_TXEN = (1 << 8),
    CR_UARTEN = (1 << 0),
};

enum line_control {
    LCR_FEN = (1 << 4),
    LCR_STP2 = (1 << 3),   
};

typedef struct pl011 {
  uint64 base_address;
  uint64 clock;
  uint32 baud_rate;
  uint32 data_bits;
  uint32 stop_bits;
} pl011;

int pl011_setup_qemu(pl011 *dev);
int pl011_send_qemu(pl011 *dev, const char *data);





