#ifndef SYSCALL_H
#define SYSCALL_H

#include <stddef.h>
#include "types.h"

/* System call numbers correspond to system calls that are handled in trap.c (trap handler) */
#define SYS_EXIT   0
#define SYS_YIELD  1
#define SYS_WRITE  2
#define SYS_READ   3
#define SYS_GETPID 4

int handle_sys_exit(int status);
void handle_sys_yield();
ssize_t handle_sys_write(int fd, const void* buf, size_t len);
ssize_t handle_sys_read(int fd, void* buf, size_t len);
int handle_getpid();

#endif 
