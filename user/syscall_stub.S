/* user/syscall_stub.S */

.macro SYSCALL name, num
.global \name 
\name:
    mov x8, #\num
    svc #0
    ret 
.endm 

SYSCALL sys_exit,   0
SYSCALL sys_yield,  1
SYSCALL sys_write,  2
SYSCALL sys_read,   3
SYSCALL sys_getpid, 4
SYSCALL sys_fork,   5
SYSCALL sys_wait,   6
SYSCALL sys_sbrk,   7
SYSCALL sys_close,  8
SYSCALL sys_pipe,   9
