#include "user_libc.h"
#include "user_syscall.h"
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>

static const char hex_values[] = {'0', '1', '2', '3', '4', '5',
                '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

/* syscalls */
void  exit(int status) { sys_exit(status); }
void  yield() { sys_yield(); }
int   write(int fd, const void* buf, size_t len) { return sys_write(fd, buf, len); }
int   read(int fd, void* buf, size_t len) { return sys_read(fd, buf, len); }
int   getpid() { return sys_getpid(); }
int   fork() { return sys_fork(); }
int   wait(int* status) { return sys_wait(status); }
void* sbrk(int incr) { return sys_sbrk(incr); }
int   close(int fd) { return sys_close(fd); }
int   pipe(int p[]) { return sys_pipe(p); }
int   open(const char* path, int flags) { return sys_open(path, flags); }
int   mkdir(const char* path) { return sys_mkdir(path); }
int   unlink(const char* path) { return sys_unlink(path); }
int   exec(const char* path, char* const argv[]) { return sys_exec(path, argv); }
int   chdir(const char* path) { return sys_chdir(path); }
int   getdents(int fd, void* buf, size_t len) { return sys_getdents(fd, buf, len); }
int   getcwd(void* buf, size_t len) { return sys_getcwd(buf, len); }
int   dup2(int oldfd, int newfd) { return sys_dup2(oldfd, newfd); }

size_t strlen(const char *str) {
  size_t len = 0;
  while (str[len]) {
    len++;
  }
  return len;
}

int strcmp(const char* s1, const char* s2) {
  if (!s1) return -1;
  if (!s2) return 1;

  int i = 0;
  while (s1[i] != '\0' && s2[i] != '\0') {
    if (s1[i] != s2[i]) {
      return s1[i] - s2[i];
    }
    i++;
  }
  return s1[i] - s2[i];
}

void *memcpy(void *restrict dst, const void *restrict src, size_t n) {
  unsigned char *dst_copy = (unsigned char *)dst;
  const unsigned char *src_copy = (const unsigned char *)src;
  for (size_t i = 0; i < n; ++i) {
    dst_copy[i] = src_copy[i];
  }
  return dst;
}

void *memmove(void *dst, const void *src, size_t n) {
  unsigned char *dst_copy = (unsigned char *)dst;
  const unsigned char *src_copy = (const unsigned char *)src;
  if (dst_copy > src_copy && dst_copy < src_copy + n) { // overlap
    for (size_t i = n; i != 0; i--) {
      dst_copy[i - 1] = src_copy[i - 1];
    }
  } else { // non-overlap
    for (size_t i = 0; i < n; ++i) {
      dst_copy[i] = src_copy[i];
    }
  }
  return dst;
}

void *memset(void *bufptr, int v, size_t len) {
  unsigned char value = (unsigned char)v;
  unsigned char *buf = (unsigned char *)bufptr;
  for (size_t i = 0; i < len; ++i) {
    buf[i] = value;
  }
  return bufptr;
}

int memcmp(const void *s1, const void *s2, size_t len) {
  const unsigned char *str_1 = (const unsigned char *)s1;
  const unsigned char *str_2 = (const unsigned char *)s2;
  for (size_t i = 0; i < len; ++i) {
    if (str_1[i] != str_2[i]) {
      return str_1[i] - str_2[i];
    }
  }

  return 0;
}

static void reverse_buffer(char* buf, int buf_len, int start) {
  int l = start, r = buf_len;
  while (l <= r) {
    char temp = buf[l];
    buf[l] = buf[r];
    buf[r] = temp;
    l++;
    r--;
  }
}

int printf(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  while (*format != '\0') {
    char character = *format;

    switch (character) {
      case '%': {
        format++;
        int precision = -1;
        if (*format == '.') {
          format++;
          precision = 0;
          while (*format >= '0' && *format <= '9') {
            precision = precision * 10 + (*format - '0');
            format++;
          }
        }
        switch (*format) {
          case 'd': // int
          case 'i': { // int  
            int num = va_arg(ap, int);
            char num_buffer[12];
            bool negative = false;
            int i = 0;

            if (num == 0) { // empty case
              write(stdout, "0", 1);
              break;
            } 
            
            if (num < 0) { // negative case
              negative = true;
              num_buffer[i++] = '-';
            }

            unsigned int unum = (negative) ? -(unsigned int)num : (unsigned int)num;
            while (unum) {
              char digit = unum % 10;
              num_buffer[i++] = digit + '0';
              unum /= 10;
            }

            num_buffer[i] = '\0';
            int buffer_len = i - 1;
            reverse_buffer(num_buffer, buffer_len, (negative) ? 1 : 0);
            write(stdout, num_buffer, i);
            break;
          }
          case 's': { // string
            const char* str = va_arg(ap, char*);
            int len = strlen(str);
            if (precision >= 0 && precision < len)
                len = precision;
            write(stdout, str, len);
            break;
          }
          case 'x': { // hex
            unsigned int num = va_arg(ap, unsigned int);
            char hex_buffer[9]; 
            if (num == 0) {
              write(stdout, "0", 1);
              break;
            }

            int i = 0;
            while (num) { 
              int hex = num % 16; 
              hex_buffer[i++] = hex_values[hex];
              num /= 16;
            }

            hex_buffer[i] = '\0';
            int buffer_len = i - 1;
            reverse_buffer(hex_buffer, buffer_len, 0);
            write(stdout, hex_buffer, i);
            break;
          }
          case 'l': {
            if (*(format + 1) == 'x') {
              format++;
              uint64 num = va_arg(ap, uint64);
              char hex_buffer[17];
              if (num == 0) {
                write(stdout, "0", 1);
                break;
              }

              int i = 0;
              while (num) {
                int hex = num % 16;
                hex_buffer[i++] = hex_values[hex];
                num /= 16;
              }

              hex_buffer[i] = '\0';
              int buffer_len = i - 1;
              reverse_buffer(hex_buffer, buffer_len, 0);
              write(stdout, hex_buffer, i);
            }
            break;
          }
          
          case 'p': { // pointer
            void* p = va_arg(ap, void*);
            uintptr_t ptr_val = (uintptr_t)p; 
            char ptr_buffer[19] = {'0','x'};
            int i = 2;

            if (ptr_val == 0) {
              ptr_buffer[i++] = '0';
              ptr_buffer[i] = '\0';
              write(stdout, ptr_buffer, 19);
              break;
            }

            while (ptr_val) { 
              int hex = ptr_val % 16; 
              ptr_buffer[i++] = hex_values[hex];
              ptr_val /= 16;
            }

            ptr_buffer[i] = '\0';
            int buffer_len = i - 1;
            reverse_buffer(ptr_buffer, buffer_len, 2);
            write(stdout, ptr_buffer, buffer_len);
          } break;

          case 'c' : {
            char c = va_arg(ap, int);
            write(stdout, &c, 1);
          }
            break;

          case '%': // display % symbol
            write(stdout, "%", 1);
            break;

          default: // format not specified
            write(stdout, "%", 1);
            write(stdout, format, 1);
            break;
        } 
        format++;
        break;

      default: // plain characters
        write(stdout, &character, 1);
        format++;
        break;
      }
    }
  }
  va_end(ap);
  return 0;
}

bool isspace(int c) {
  if (c == ' ' || c == '\n' || c == '\t') {
    return true;
  }
  return false;
}