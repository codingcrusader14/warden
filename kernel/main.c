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

void idle(void* args) {
 (void)args;
 while (1) {
  
 }
}


void kernel_main(void) {
    uart_init();
    pmm_init(QEMU_DRAM_START, QEMU_DRAM_END);
    vmm_init();
    gic_init();
    timer_init();
    scheduler_init();
    task_t* a = task_create((void (*)(void*))user_entry,NULL, NORMAL_TASK);
    task_t* b = task_create(idle, NULL, NORMAL_TASK);
    scheduler_add(a);
    scheduler_add(b);
    scheduler_start();
}
