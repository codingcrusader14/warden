#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

#include <stddef.h>

void sys_exit(int status);
void sys_yield();
void sys_write(const char* buf, size_t len);

#endif
