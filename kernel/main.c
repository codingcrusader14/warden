#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include "../drivers/qemu/pl011.h" 
#include "global.h"
#include "libk/includes/stdio.h"
#include "libk/includes/stdlib.h"
#include "../drivers/qemu/timer.h"
#include "../drivers/qemu/gic.h"
#include "trap.h"
#include "vmm.h"
#include "pmm.h"
#include "schedule.h" 
#include "process.h"

void task_a(void) {
    for (int i = 0; i < 100; i++) {
        kprintf("Task A: %d\n", i);
        yield();
    }
    // returns here, trampoline calls kexit
    kprintf("Global tick value %d\n", global_tick);
}

void task_b(void) {
    for (int i = 0; i < 100; i++) {
        kprintf("Task B: %d\n", i);
    }
    kprintf("Global tick value %d\n", global_tick);
}

void task_c(void) {
    for (int i = 0; i < 100; i++) {
        kprintf("Task C: %d\n", i);
    }
    // returns here, trampoline calls kexit
    kprintf("Global tick value %d\n", global_tick);
}

void task_d(void) {
    for (int i = 0; i < 100; i++) {
        kprintf("Task D: %d\n", i);
        yield();
    }
    kprintf("Global tick value %d\n", global_tick);
}

void kernel_main(void) {
   uart_init();
   pmm_init(QEMU_DRAM_START, QEMU_DRAM_END);
   vmm_init();
   gic_init();
   timer_init();
   struct task* a = task_create(task_a);
   struct task* b = task_create(task_b);
    struct task* c = task_create(task_c);
   struct task* d = task_create(task_d);
   scheduler_add(a);
   scheduler_add(b);
   scheduler_add(c);
   scheduler_add(d);
   scheduler_start();
}
