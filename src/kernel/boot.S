.section ".text.boot"
.global _entry

_entry:
        // Set vector table base address
        ldr x0, = vtable_start
        msr VBAR_EL1, x1
        dsb sy
        isb   
                        // x0 contains device tree address from bootloader
        mov x19, x0
        ldr x0, = stack_top
        mov sp, x0
        bl kernel_main
halt:
        wfe
        b halt


