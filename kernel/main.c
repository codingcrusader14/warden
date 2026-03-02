#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include "../drivers/qemu/pl011.h" 
#include "libk/includes/stdio.h"
#include "libk/includes/stdlib.h"
#include "../drivers/qemu/timer.h"
#include "../drivers/qemu/gic.h"
#include "vmm.h"
#include "pmm.h"
#include "schedule.h" 
#include "process.h"

extern void context_switch(struct task_context* prev, struct task_context* next);
extern void sched_start(struct task_context* current);

void yield() {
  schedule();
}

void print_a() {
  while (1) {
    kprintf("Task A is running\n");
    yield();
  }
}

void print_b() {
  while (1) {
    kprintf("Task B is running\n");
    yield();
  }
}

void print_c() {
      kprintf("Entered task C\n");

  while (1) {
    kprintf("Task C is running\n");
    yield();
  }
}



void kernel_main(void) {
   uart_init();
   pmm_init(QEMU_DRAM_START, QEMU_DRAM_END);
   vmm_init();
   gic_init();
   timer_init();
   struct task* a = task_create(print_a);
   struct task* b = task_create(print_b);
   struct task* c = task_create(print_c);
   scheduler_add(a);
   scheduler_add(b);
   scheduler_add(c);
   kprintf("stack a %p\n", a->stack_base);
   kprintf("stack b %p\n", b->stack_base);
   kprintf("stack c %p\n", c->stack_base);
   sched_start(&current_task->context_registers);
}
