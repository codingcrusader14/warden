#include "mmu_defs.h"

.section ".text.boot"
.global _entry

_entry:
        mov x1, x0 // x0 contains device tree address from bootloader
        ldr x2, = boot_pagetables
        ldr x5, = boot_pagetables + 0x4000 

fill_pagetables:
        str xzr, [x2]
        add x2, x2, #8
        cmp x2, x5
        blt fill_pagetables

        ldr x3, = boot_ttbr1_l1
        ldr x4, =BOOT_TABLE_ATTR
        orr x3, x3, x4

        ldr x4, = boot_ttbr1_l0
        str x3, [x4]

        ldr x3, = boot_ttbr1_l1 
        ldr x4, =BOOT_DEVICE_BLOCK_ATTR
        str x4, [x3]

        ldr x4, =BOOT_NORMAL_BLOCK_ATTR
        ldr x5, =KERNEL_MEMORY
        orr x4, x4, x5
        str x4, [x3, #8]

        ldr x3, = boot_ttbr0_l1
        ldr x4, =BOOT_TABLE_ATTR
        orr x3, x3, x4
        
        ldr x4, = boot_ttbr0_l0 
        str x3, [x4]
        ldr x3, = boot_ttbr0_l1 
        ldr x4, =BOOT_NORMAL_BLOCK_ATTR
        ldr x5, =KERNEL_MEMORY
        orr x4, x4, x5
        str x4, [x3, #8]

        ldr x0, =boot_ttbr0_l0


        // configure mmu 
        msr ttbr0_el1, x0
        ldr x1, =boot_ttbr1_l0
        msr ttbr1_el1, x1
        ldr x2, =MAIR_VALUE
        msr mair_el1, x2
        ldr x3, =TCR_VALUE
        msr tcr_el1, x3
        isb
        mrs x1, sctlr_el1
        orr x1, x1, #1
        msr sctlr_el1, x1
        isb


        ldr x0, =boot_high 
        br x0

.section ".text"
.globl boot_high 

boot_high:
        ldr x0, = vtable_start // Set vector table base address
        msr VBAR_EL1, x0
        dsb sy
        isb   
                    
        ldr x2, = stack_top
        mov sp, x2

        ldr x0, =_bss_start
        ldr x1, =_bss_end

bss_clear:
        cmp x0, x1
        b.ge bss_done
        str xzr, [x0]
        add x0, x0, #8
        b bss_clear

bss_done:

        bl kernel_main

halt:
        wfe
        b halt


