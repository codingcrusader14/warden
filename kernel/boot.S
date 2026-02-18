.section ".text.boot"
.global _entry

_entry:
        mov x1, x0 // x0 contains device tree address from bootloader
        ldr x0, = vtable_start // Set vector table base address
        msr VBAR_EL1, x0
        dsb sy
        isb   
                    
        ldr x2, = stack_top
        mov sp, x2
        bl kernel_main
halt:
        wfe
        b halt


