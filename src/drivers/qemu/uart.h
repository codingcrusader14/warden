/*
 * UART driver for Armv8-A (QEMU virt)
 * 
 * Provides serial output and input to the console
 * Hardware : Arm PrimeCell UART PL011
 */
 
#include "../../kernel/types.h"

static constexpr uint64 UART_BASE = 0x9000000;

static constexpr uint8 UART_INTERRUPT_TYPE = 0;
static constexpr uint8 UART_INTERRUPT_ID = 1;
static constexpr uint8 UART_INTERRUPT_FLAGS = 4;

#define UART_DATA_REGISTER                           (*(volatile uint16*) (UART_BASE + 0x000))
#define UART_RECIEVE_STATUS_REGISTER                 (*(volatile uint16*) (UART_BASE + 0x004))
#define UART_ERROR_CLEAR_REGISTER                    (*(volatile uint16*) (UART_BASE + 0x004))
#define UART_FLAG_REGISTER                           (*(volatile uint16*) (UART_BASE + 0x018))
#define UART_IRDA_LOW_POWER_COUNTER_REGISTER         (*(volatile uint16*) (UART_BASE + 0x020))
#define UART_INTEGER_BAUD_RATE_REGISTER              (*(volatile uint16*) (UART_BASE + 0x024))
#define UART_FRACTIONAL_BAUD_RATE_REGISTER           (*(volatile uint16*) (UART_BASE + 0x028))
#define UART_LINE_CONTROL_REGISTER                   (*(volatile uint16*) (UART_BASE + 0x02C))
#define UART_CONTROL_REGISTER                        (*(volatile uint16*) (UART_BASE + 0x030))
#define UART_INTERRUPT_FIFO_LEVEL_SELECT_REGISTER    (*(volatile uint16*) (UART_BASE + 0x034))
#define UART_INTERRUPT_MASK_SET_CLEAR_REGISTER       (*(volatile uint16*) (UART_BASE + 0x038))
#define UART_RAW_INTERRUPT_STATUS_REGISTER           (*(volatile uint16*) (UART_BASE + 0x03C))
#define UART_MASKED_INTERRUPT_STATUS_REGISTER        (*(volatile uint16*) (UART_BASE + 0x040))
#define UART_INTERRUPT_CLEAR_REGISTER                (*(volatile uint16*) (UART_BASE + 0x044))
#define UART_DMA_CONTROL_REGISTER                    (*(volatile uint16*) (UART_BASE + 0x048))

/* Flags for flag register */

#define UART_TX_FIFO_FULL                     ((UART_FLAG_REGISTER >> 5) & 1) // 1 = FIFO full
#define UART_RX_FIFO_EMPTY                    ((UART_FLAG_REGISTER >> 4) & 1) // 1 = FIFO empty
#define UART_BUSY                             ((UART_FLAG_REGISTER >> 3) & 1) // 1 = Transmiting

/* Baud rates */
// #define INT_BAUD_RATE 


/* UART initlization process */
static inline void check_transmission() {
 while (UART_BUSY) {
  ;
 }
}

static inline void disable_uart() {
 UART_CONTROL_REGISTER &= ~(1U << 0);
}

static inline void config_integer_baud_rate(uint16 baud) {
 UART_INTEGER_BAUD_RATE_REGISTER = baud;
}

static inline void enable_uart() {
 UART_CONTROL_REGISTER |= (1U << 0);
}

/* Interface */
void uart_init(); 
void uart_put_c(char c);
int32 uart_get_c();





