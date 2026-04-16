#ifndef LIB_C
#define LIB_C

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   2
#define O_CREAT  4

#define MAX_DIRECT_ENTRIES 64

typedef struct {
    char name[11];
    uint8_t attr;
    uint32_t size;
} dirent;


#define stdin  0
#define stdout 1
#define stderr 2

void  exit(int status);
void  yield();
int   write(int fd, const void* buf, size_t len);
int   read(int fd, void* buf, size_t len);
int   getpid();
int   fork();
int   wait(int* status);
void* sbrk(int incr);
int   close(int fd);
int   pipe(int p[]);
int   open(const char* path, int flags);
int   mkdir(const char* path);
int   unlink(const char* path);
int   exec(const char* path, char* const argv[]);
int   chdir(const char* path);
int   getdents(int fd, void* buf, size_t len);
int   getcwd(void* buf, size_t len);

size_t strlen(const char*);
int strcmp(const char* s1, const char* s2);
void* memcpy(void* , const void* , size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
int memcmp(const void*, const void*, size_t);
int printf(const char* format, ...);

bool isspace(int c);

#endif