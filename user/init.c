#include "user_syscall.h"


void _start() {
  sys_write(1, "init started\n", 13);
  sys_exec("/hello.elf");
  sys_exit(1);
}
