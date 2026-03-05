#include "process.h"
#include "schedule.h"
#include "../drivers/qemu/timer.h"
#include "libk/includes/stdlib.h"
#include "libk/includes/string.h"
#include "libk/includes/stdio.h"
#include "global.h"

uint64 next_pid = 1;

void kexit() {
  current_task->state = DEAD;
  int ret = schedule();
  if (ret == -1) {
      kprintf("All tasks complete. Halting.\n");
      disable_interrupts();
      while (1);
    }
    kprintf("Panic: kexit should not return.\n");
    while (1);
}

void yield() {
  current_task->state = READY;
  schedule();
}

void task_trampoline() {
  enable_interrupts();
  void (*entry)(void) = (void (*)(void))current_task->ctx.x19;
  entry();
  kexit();
}

task_t* task_create(void (*entry)(void), uint64 ticket_level) {
  task_t* new_task = kmalloc(sizeof(task_t));
  memset(new_task, 0, sizeof(task_t));
  new_task->stack_base = kmalloc(STACK_SIZE);
  new_task->pid = next_pid++;
  new_task->state = READY;
  new_task->tickets = ticket_level;
  new_task->stride = (STRIDE_BASE / new_task->tickets);
  new_task->pass = 0;
  new_task->remain = new_task->stride;
  new_task->scheduler_tick = 0;

  uint64 sp_top = ((uint64) new_task->stack_base + STACK_SIZE) & ~0xF;

  memset(&new_task->ctx, 0, sizeof(struct context));
  new_task->ctx.x30 = (uint64)task_trampoline;
  new_task->ctx.x19 = (uint64)entry; 
  new_task->ctx.sp = sp_top;

  return new_task;
}

void task_free(task_t* t){
  kfree(t->stack_base);
  kfree(t);
}
