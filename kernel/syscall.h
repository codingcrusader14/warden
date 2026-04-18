#ifndef SYSCALL_H
#define SYSCALL_H

#include <stddef.h>
#include "types.h"
#include "file.h"

#define MAX_ARGS 16
#define MAX_ARG_LEN 64

/* System call numbers correspond to system calls that are handled in trap.c (trap handler) */
#define SYS_EXIT   0
#define SYS_YIELD  1
#define SYS_WRITE  2
#define SYS_READ   3
#define SYS_GETPID 4
#define SYS_FORK   5 
#define SYS_WAIT   6
#define SYS_SBRK   7
#define SYS_CLOSE  8
#define SYS_PIPE   9
#define SYS_OPEN   10
#define SYS_MKDIR  11
#define SYS_UNLINK 12
#define SYS_EXEC   13
#define SYS_CHDIR  14
#define SYS_GETDENTS  15
#define SYS_GETCWD  16
#define SYS_DUP2    17

int handle_sys_exit(int status);
void handle_sys_yield();
ssize_t handle_sys_write(int fd, const void* buf, size_t len);
ssize_t handle_sys_read(int fd, void* buf, size_t len);
pid_t handle_getpid();
pid_t handle_fork();
pid_t handle_wait(int* status);
void* handle_sbrk(int incr);
int handle_close(int fd);
int handle_pipe(int p[]);
int handle_open(const char* path, int flags);
int handle_mkdir(const char* path); // create a new directory
int handle_unlink(const char* path); // remove a file
int handle_exec(const char* path, char* const argv[]); // loads a new file and executes with arguments
int handle_chdir(const char* path); // change directory
int handle_getdents(int fd, void* buffer, size_t bufsize); // reads current directory entries
int handle_getcwd(void* buffer, size_t bufsize); // gets current working directory
int handle_dup2(int oldfd, int newfd); // duplicates file descriptor closes old one

#endif 
