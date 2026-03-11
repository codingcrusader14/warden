#include "syscall.h" 
#include "mmu_defs.h"
#include "process.h" 
#include "libk/includes/stdio.h"
#include "../drivers/qemu/pl011.h"

void handle_sys_exit(int status) {
  kprintf("Process exited with status %d\n", status);
  kexit();
}

void handle_sys_yield() {
  yield();
}

void handle_sys_write(const char* buf, size_t len) {
  if ((uint64) buf > KERNEL_HIGH_VA_ADDRESS || (uint64) buf + len > KERNEL_HIGH_VA_ADDRESS){ // check if user is accessing kernel memory
    return;
  }

  for (size_t i = 0; i < len; ++i) {
    put_char(buf[i]);
  }
}
