#ifndef SYSCALL_H
#define SYSCALL_H

#include <stddef.h>

/* System call numbers correspond to system calls that are handled in trap.c (trap handler) */
#define SYS_EXIT  0
#define SYS_YIELD 1
#define SYS_WRITE 2

void handle_sys_exit(int status);
void handle_sys_yield();
void handle_sys_write(const char* buf, size_t len);

#endif 
