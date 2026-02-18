#include "trap.h" 
#include "libk/includes/stdio.h"


void kerneltrap(struct trapframe *tf) {
  kprintf("Exception (ESR): %p\n", tf->esr_el1);
  kprintf("Faulting Address (FAR): %p\n", tf->far_el1);
  kprintf("Return Address (ELR): %p\n", tf->elr_el1);
}
