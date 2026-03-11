#include "user_syscall.h"

void _start() {
  sys_write("Hello from EL0!\n", 16);
  sys_exit(0);
}
