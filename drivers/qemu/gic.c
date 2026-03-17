#include "gic.h"
#include "../../kernel/types.h"

static inline uint32 gicd_read(enum distributor_regs offset) {
    return *(volatile uint32*) (QEMU_DISTRIBUTOR_BASE + offset);
}

static inline void gicd_write(enum distributor_regs offset, uint32 value) {
    *(volatile uint32*) (QEMU_DISTRIBUTOR_BASE + offset) = value;
}

static inline uint32 gicc_read(enum cpu_regs offset) {
    return *(volatile uint32*) (QEMU_CPU_INTERFACE_BASE+ offset);
}

static inline void gicc_write(enum cpu_regs offset, uint32 value) {
    *(volatile uint32*) (QEMU_CPU_INTERFACE_BASE + offset) = value;
}

static inline void gic_set_priority(uint32 interrupt_id, uint32 priority) {
  const uint32 shift_prio[] = {0, 8, 16, 24};
  uint32 index = (interrupt_id / 4);
  uint32 offset = (INTERRUPT_PRIORITY_REGISTERS + (4 * index));
  uint32 byte_offset = interrupt_id % 4;
  uint32 gicd_reg = gicd_read(offset);
  uint32 cleared = gicd_reg & ~(0xFF << shift_prio[byte_offset]);
  uint32 new_val = cleared | (priority << shift_prio[byte_offset]);
  gicd_write(offset, new_val);
}

uint32 read_interrupt_ack() {
  uint32 interrupt_ack_reg = gicc_read(INTERRUPT_ACKNOWLEDGE_REGISTER);
  return interrupt_ack_reg & 0x3FF;
}

void write_end_of_interrupt(uint32 interrupt_id) {
  gicc_write(END_OF_INTERRUPT_REGISTER, interrupt_id);
}

static inline void gic_enable_irq(uint32 interrupt_id) {
  uint32 reg_offset = INTERRUPT_SET_ENABLE + (4 * (interrupt_id / 32));
  gicd_write(reg_offset, (1 << (interrupt_id % 32)));
}

static inline void gic_set_target(uint32 interrupt_id, uint32 cpu_mask) {
  uint32 reg_offset = INTERRUPT_TARGET + (interrupt_id & ~3);
  uint32 byte_shift = (interrupt_id % 4) * 8;
  uint32 val = gicd_read(reg_offset);
  val &= ~(0xFF << byte_shift);
  val |= (cpu_mask << byte_shift);
  gicd_write(reg_offset, val);
}

void gic_init() {
  gicd_write(DISTRIBUTOR_CONTROL_REGISTER, ENABLE_FORWARDING);

  gic_enable_irq(TIMER_PPI_ID);
  gic_set_priority(TIMER_PPI_ID, 0x00); 

  gic_enable_irq(UART);
  gic_set_priority(UART, 0x00);
  gic_set_target(UART, 0x01); // CPU 0

  gicc_write(INTERRUPT_PRIORITY_MASK_REGISTER, PRIORITY_FILTER);
  gicc_write(CPU_INTERFACE_CONTROL_REGISTER, ENABLE_SIGNALING);
}
