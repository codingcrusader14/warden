#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include "../drivers/qemu/pl011.h" 
#include "../drivers/qemu/timer.h"
#include "../drivers/qemu/gic.h"
#include "libk/includes/stdio.h"
#include "vmm.h"
#include "pmm.h"
#include "schedule.h" 
#include "process.h"

void task_a(void) {
    for (size_t i = 0; i < 10000000; ++i) {
      kprintf("Task A\n");
    }
}

void task_b(void) {
    for (size_t i = 0; i < 1000000; ++i) {
      kprintf("Task B\n");
    }
}

void task_c(void) {
    for (size_t i = 0; i < 100000000; ++i) {
      kprintf("Task C\n");
    }
}

void reaper_task(void) {
  kprintf("Tasks Cleanup\n");
  while (1) {
   cleanup_dead_task();
  }
  kprintf("Tasks Complete\n");
}

void kernel_main(void) {
   uart_init();
   pmm_init(QEMU_DRAM_START, QEMU_DRAM_END);
   vmm_init();
   gic_init();
   timer_init();
   scheduler_init();
   task_t* a = task_create(task_a, NORMAL_TASK);
   task_t* b = task_create(task_b, NORMAL_TASK);
   task_t* c = task_create(task_c, NORMAL_TASK);
   task_t* reaper = task_create(reaper_task, REAPER_TASK);
   scheduler_add(a);
   scheduler_add(b);
   scheduler_add(c);
   scheduler_add(reaper);
   scheduler_start();
  


}
