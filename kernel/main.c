#include <stdint.h>
#include <stddef.h>
#include "../drivers/qemu/pl011.h" 
#include "../drivers/qemu/timer.h"
#include "../drivers/qemu/gic.h"
#include "libk/includes/stdio.h"
#include "libk/includes/stdlib.h"
#include "libk/includes/string.h"
#include "mmu_defs.h"
#include "syscall.h"
#include "vmm.h"
#include "pmm.h"
#include "schedule.h" 
#include "process.h"
#include "../user/user_syscall.h"
#include "console.h"
#include "../drivers/virtio.h"
#include "../fs/fat32.h"

void idle(void* args) {
    (void)args;
    while (1) {}
}

void kernel_main(void) {
    uart_init();
    pmm_init(QEMU_DRAM_START, QEMU_DRAM_END);
    vmm_init();
    gic_init();
    timer_init();
    virtio_disk_init();
    init_disk_bpb();
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
