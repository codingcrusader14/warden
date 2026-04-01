#include <stdint.h>
#include <stddef.h>
#include "../drivers/qemu/pl011.h" 
#include "../drivers/qemu/timer.h"
#include "../drivers/qemu/gic.h"
#include "libk/includes/stdio.h"
#include "libk/includes/stdlib.h"
#include "syscall.h"
#include "vmm.h"
#include "pmm.h"
#include "schedule.h" 
#include "process.h"
#include "../user/user_syscall.h"
#include "console.h"
#include "../drivers/virtio.h"

void idle(void* args) {
    (void)args;
    kprintf("idle: about to read disk\n");
    char *buf = kmalloc(512);
    kprintf("idle: buf allocated at %p\n", buf);
    int ret = virtio_disk_rw(buf, 0, 0);
    kprintf("virtio read status: %d\n", ret);
    kprintf("first bytes: %x %x %x\n", (uint8)buf[0], (uint8)buf[1], (uint8)buf[2]);
    kfree(buf);
    while (1) {}
}

void kernel_main(void) {
    uart_init();
    pmm_init(QEMU_DRAM_START, QEMU_DRAM_END);
    vmm_init();
    gic_init();
    timer_init();
    virtio_disk_init();
    scheduler_init();
    task_t* b = kernel_task_create(idle, NULL, NORMAL_TASK);
    task_t* a = task_create((void (*)(void*))user_entry,NULL, NORMAL_TASK);
    file *f_stdin = file_alloc(FILE_CONSOLE, &console_ops, NULL);
    file *f_stdout = file_alloc(FILE_CONSOLE, &console_ops, NULL);
    file *f_stderr = file_alloc(FILE_CONSOLE, &console_ops, NULL);
    if (!f_stdin || !f_stdout || !f_stderr) {
      kprintf("Error: file descriptors failed to allocate\n");
      return;
    }
    a->fd_table[0] = f_stdin;
    a->fd_table[1] = f_stdout;
    a->fd_table[2] = f_stderr;
    scheduler_add(a);
    scheduler_add(b);
    scheduler_start();
}
