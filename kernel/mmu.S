
#include "mmu_defs.h"

.globl tlb_invalidate
.balign 8


tlb_invalidate:
        dsb ishst
        tlbi vale1is, x0
        dsb ish
        isb

        ret




