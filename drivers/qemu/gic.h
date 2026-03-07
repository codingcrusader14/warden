#ifndef QEMU_GIC_H
#define QEMU_GIC_H

#include "../../kernel/types.h"
#include "../../kernel/mmu_defs.h"

#define QEMU_DISTRIBUTOR_BASE PA_TO_KVA(0x8000000UL)
#define QEMU_CPU_INTERFACE_BASE PA_TO_KVA(0x8010000UL)

#define TIMER_PPI_ID 30
#define PRIORITY_FILTER 0xFF
#define ENABLE_TIMER_BIT (1 << TIMER_PPI_ID)

enum distributor_regs {
  DISTRIBUTOR_CONTROL_REGISTER    = 0x000, 
  INTERRUPT_SET_ENABLE_REGISTERS  = 0x100,
  INTERRUPT_PRIORITY_REGISTERS    = 0x400,
};

enum cpu_regs {
  CPU_INTERFACE_CONTROL_REGISTER        = 0x000,
  INTERRUPT_PRIORITY_MASK_REGISTER      = 0x004,
  INTERRUPT_ACKNOWLEDGE_REGISTER        = 0x00C, // RO [9:0] Interrupt ID, [12:10] CPU ID
  END_OF_INTERRUPT_REGISTER             = 0x010, // WO [9:0] End of Interrupt ID, [12:10] CPU ID
};

enum control_regs {
  ENABLE_FORWARDING = (1 << 0), // enable interrupt forwarding
  ENABLE_SIGNALING  = (1 << 0), // enable signaling of interrupts
};


void gic_init();
uint32 read_interrupt_ack();
void write_end_of_interrupt(uint32 interrupt_id);

#endif
