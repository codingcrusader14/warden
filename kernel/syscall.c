#include "syscall.h"
#include "../drivers/qemu/pl011.h"
#include "libk/includes/stdio.h"
#include "mmu_defs.h"
#include "process.h"
#include "schedule.h"
#include "vmm.h"

int handle_sys_exit(int status) {
  kprintf("Process exited with status %d\n", status);
  kexit();
  return 0;
}

void handle_sys_yield() { yield(); }

ssize_t handle_sys_write(int fd, const void* buf, size_t len) {
  if (!buf || len == 0) 
    return 0;

  switch (fd) {
    case 1: {
      const char* cbuf = (const char*)buf;
      ssize_t bytes_written = 0;
      for (size_t i = 0; i < len; ++i) {
        put_char(cbuf[i]);
        bytes_written++;
      }
      return bytes_written;
    }

    default: {
      return 0;
    }
  }
}

ssize_t handle_sys_read(int fd, void *buf, size_t len) {
  if (!buf || len == 0)
    return -1;

  switch (fd) {
  case 0: { // stdin
    unsigned char kbuf[256];
    len = (len < 256) ? len : 256;

    kbuf[0] = uart_read();
    ssize_t chars_read = 1;

    for (size_t i = 1; i < len; ++i) {
      int character = uart_try_read();
      if (character < 0)
        break;
      kbuf[i] = character;
      chars_read++;
    }

    if (copy_to_user((pte_t *)PA_TO_KVA(current_task->pgd), buf, kbuf,
                     chars_read) < 0)
      return -1;

    return chars_read;
  } break;

  default: { // not implemented
    return -1;
  }
  }
}

int handle_getpid() { return current_task->pid; }
