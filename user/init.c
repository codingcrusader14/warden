#include "user_syscall.h"

void _start() {
  char buf[64];
  int pos = 0; 
  sys_write(1, "Type something: ", 17);

  while (pos < 63) {
    ssize_t n = sys_read(0, &buf[pos],1);
    if (n <= 0) break;
    if (buf[pos] == '\r' || buf[pos] == '\n') break;
    pos += n;
  }

  buf[pos] = '\0';
  sys_write(1, buf, pos);
  sys_write(1, "\n", 1);
  sys_exit(0);
}
