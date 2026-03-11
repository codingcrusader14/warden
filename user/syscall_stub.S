/* user/syscall_stub.S */

.macro SYSCALL name, num
.global \name 
\name:
    mov x8, #\num
    svc #0
    ret 
.endm 

SYSCALL sys_exit,  0
SYSCALL sys_yield, 1
SYSCALL sys_write, 2
