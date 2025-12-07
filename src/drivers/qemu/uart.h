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

void uartputc(char c);





