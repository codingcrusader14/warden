#include "trap.h" 
#include "libk/includes/stdio.h"


void kernelvec_sync(struct trapframe *tf) {
  uint64_t ec = (tf->esr_el1 >> 26) & (0x3F);
  kprintf("Exception SYNC Recieved!\n");
  kprintf("Exception (ESR EC Code) : %d\n", ec);
  kprintf("Faulting Address (FAR): %p\n", tf->far_el1);
  kprintf("Return Address (ELR): %p\n", tf->elr_el1);
  while (1) {}
}
void kernelvec_irq(struct trapframe *tf) {
  kprintf("Exception IRQ Recieved!\n");
  kprintf("Faulting Address (FAR): %p\n", tf->far_el1);
  while (1) {}
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
