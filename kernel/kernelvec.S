.globl sync_exception
.globl irq_exception
.globl fiq_exception
.globl system_exception
.globl save_regs
.globl restore_regs
.balign 8

save_regs:
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
        mrs x3, ESR_EL1
        mrs x4, FAR_EL1
        stp x1, x2, [sp, #256]
        stp x3, x4, [sp, #272]
        str x0, [sp, #248]
        ret 

restore_regs:
        ldp x2, x1, [sp, #256]
        ldr x0,     [sp, #248]
        msr SPSR_El1, x2 
        msr SP_EL0,   x1 
        msr ELR_EL1,  x0
        ldp x28, x29, [sp, #224]
        ldp x26, x27, [sp, #208]
        ldp x24, x25, [sp, #192]
        ldp x22, x23, [sp, #176]
        ldp x20, x21, [sp, #160]
        ldp x18, x19, [sp, #144]
        ldp x16, x17, [sp, #128]
        ldp x14, x15, [sp, #112]
        ldp x12, x13, [sp, #96]
        ldp x10, x11, [sp, #80]
        ldp x8, x9,   [sp, #64]
        ldp x6, x7,   [sp, #48]
        ldp x4, x5,   [sp, #32]
        ldp x2, x3,   [sp, #16]
        ldp x0, x1,   [sp, #0]
        ret

sync_exception:
        sub sp, sp, 288
        str x30, [sp, #240]
        bl save_regs
        mov x0, sp
        bl kernelvec_sync
        bl restore_regs 
        ldr x30, [sp, #240]
        add sp, sp, 288
        eret


irq_exception:
        sub sp, sp, 288
        str x30, [sp, #240]
        bl save_regs
        mov x0, sp
        bl kernelvec_irq 
        bl restore_regs 
        ldr x30, [sp, #240]
        add sp, sp, 288
        eret

fiq_exception:
        sub sp, sp, 288
        str x30, [sp, #240]
        bl save_regs
        mov x0, sp
        bl kernelvec_fiq
        bl restore_regs 
        ldr x30, [sp, #240]
        add sp, sp, 288
        eret


system_exception:
        sub sp, sp, 288
        str x30, [sp, #240]
        bl save_regs
        mov x0, sp
        bl kernelvec_system
        bl restore_regs 
        ldr x30, [sp, #240]
        add sp, sp, 288
        eret

