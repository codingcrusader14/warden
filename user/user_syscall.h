#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

#include <stddef.h>
#include "../kernel/types.h"

int sys_exit(int status);
void sys_yield();
ssize_t sys_write(int fd, const void* buf, size_t len);
ssize_t sys_read(int fd, void* buf, size_t len);
pid_t sys_getpid();
pid_t sys_fork();
pid_t sys_wait(int* status);
void* sys_sbrk(int incr);

#endif
