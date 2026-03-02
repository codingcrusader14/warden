#include "trap.h" 
#include "libk/includes/stdio.h"
#include "../drivers/qemu/gic.h"
#include "../drivers/qemu/timer.h"
#include "global.h"

void kernelvec_sync(struct trapframe *tf) {
  uint64_t ec = (tf->esr_el1 >> 26) & (0x3F);
  kprintf("Exception SYNC Recieved!\n");
  kprintf("Exception (ESR EC Code) : %d\n", ec);
  kprintf("Faulting Address (FAR): %p\n", tf->far_el1);
  kprintf("Return Address (ELR): %p\n", tf->elr_el1);
  while (1) {}
}
void kernelvec_irq(struct trapframe* tf) {
  UNUSED(tf);
  uint32 ack_id = read_interrupt_ack();
  switch(ack_id) {
    case 30 : {
      timer_rearm();
      break;
    }
    default : 
      kprintf("This interrupt is not recongnized\n");
      break;
  }
  write_end_of_interrupt(ack_id);
}

void kernelvec_fiq(struct trapframe *tf) {
  kprintf("Exception FIQ Recieved!\n");
  kprintf("Faulting Address (FAR): %p\n", tf->far_el1);
  while (1) {}
}

void kernelvec_system(struct trapframe *tf) {
   kprintf("Exception SERROR Recieved!\n");
   kprintf("Faulting Address (FAR): %p\n", tf->far_el1);
   while (1) {}
}
