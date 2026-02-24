
#include "mmu_defs.h"

.globl configure_mmu
.balign 8

configure_mmu: 
          msr TTBR0_EL1, x0 // store translation table base register user space
          msr TTBR1_EL1, x1 // store translation table base register kernel_space
          ldr x2, =MAIR_VALUE
          msr mair_el1, x2 // store MAIR_El1
          ldr x3, =TCR_VALUE
          msr tcr_el1, x3 // store TCR_EL1

          isb

          mrs x1, sctlr_el1
          orr x1, x1, 1 // turn on MMU
          msr sctlr_el1, x1

          isb

          ret




