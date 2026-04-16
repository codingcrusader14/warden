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
int sys_close(int fd);
int sys_pipe(int p[]);
int sys_open(const char* path, int flags);
int sys_mkdir(const char* path);
int sys_unlink(const char* path);
int sys_exec(const char* path, char* const argv[]);
int sys_chdir(const char* path);
int sys_getdents(int fd, void* buf, size_t buf_len);
int sys_getcwd(void* buf, size_t buf_len);

#endif
