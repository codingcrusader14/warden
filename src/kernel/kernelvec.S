.globl kerneltrap
.globl kernelvec
.balign 8

kernelvec:
        # making room to save register state.
        
        sub sp, sp, 272

        # save caller saved registers.
        stp x0, x1, [sp, #0]
        stp x2, x3, [sp, #16]
        stp x4, x5, [sp, #32]
        stp x6, x7, [sp, #48]
        stp x8, x9, [sp, #64]
        stp x10, x11, [sp, #80]
        stp x12, x13, [sp, #96]
        stp x14, x15, [sp, #112]
        stp x16, x17, [sp, #128]
        stp x18, x19, [sp, #144]
        stp x20, x21, [sp, #160]
        stp x22, x23, [sp, #176]
        stp x24, x25, [sp, #192]
        stp x26, x27, [sp, #208]
        stp x28, x29, [sp, #224]
        mrs x0, ELR_EL1
        mrs x1, SPSR_EL1
        mrs x2, SP_EL0
        stp x30, x0, [sp, #240]
        stp x1, x2, [sp, #256]
