#include "user_syscall.h"

void _start() {
  // get current brk
  void *start = sys_sbrk(4096);

  // write to the new memory
  char *heap = (char *)start;
  const char* str = "Hello, World!\n";
  size_t i = 0;
  while (str[i]) {
    heap[i] = str[i];
    i++;
  }

  
  sys_write(1, heap, 15);
  sys_exit(0);
}
