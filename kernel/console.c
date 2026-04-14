#include "console.h"
#include "schedule.h"
#include "vmm.h"
#include "../drivers/qemu/pl011.h"
#include "../kernel/libk/includes/stdio.h"

int64 console_write(file *f, const void *buf, size_t len) {
  if (!f || !buf) return -1;

  const char* cbuf = (const char*)buf;
  ssize_t bytes_written = 0;
  for (size_t i = 0; i < len; ++i) {
    put_char(cbuf[i]);
    bytes_written++;
  }
  return bytes_written;
}

int64 console_read(file *f, void *buf, size_t len) {
  if (!f || !buf) return -1;

  unsigned char kbuf[256];
  len = (len < 256) ? len : 256;

  size_t i = 0;
  while (i < len) {
    int c = uart_read();
    if (c == '\r') c = '\n';
    put_char(c);
    kbuf[i++] = c;
    if (c == '\n') break;
  }

  if (copy_to_user((pte_t *)PA_TO_KVA(current_task->pgd), buf, kbuf, i) < 0)
    return -1;

  return i;
}

int64 console_close(file* f) {
  if (!f) return -1;
  return 0;
}

file_ops console_ops = {.write = console_write, .read = console_read, .close = console_close};
