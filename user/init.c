#include "user_syscall.h"


void _start() {
  int fd = sys_open("/hello.txt", 0);
  uint8 buffer[512];

  int bytes_read = sys_read(fd, buffer, 512);
  sys_write(1, buffer, bytes_read);
  sys_close(fd);
  sys_exit(0);
}
