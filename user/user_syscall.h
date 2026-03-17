#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

#include <stddef.h>
#include "../kernel/types.h"

int sys_exit(int status);
void sys_yield();
ssize_t sys_write(int fd, const void* buf, size_t len);
ssize_t sys_read(int fd, void* buf, size_t len);
int getpid();
#endif
