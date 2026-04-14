
#include "mmu_defs.h"

.globl tlb_invalidate
.globl tlb_invalidate_process
.globl enter_userspace
.balign 8


tlb_invalidate:
        dsb ishst
        tlbi vale1is, x0
        dsb ish
        isb

        ret

tlb_invalidate_process:
        isb
        tlbi aside1is, x0
        dsb ish
        isb

        ret

enter_userspace:
        msr ttbr0_el1, x0 
        isb
        tlbi vmalle1is
        dsb ish
        isb

        // Set return address to user entry point
        msr elr_el1, x1

        // EL0, all interrupts enabled
        mov x3, xzr
        msr spsr_el1, x3

        // User stack
        msr sp_el0, x2


        eret


