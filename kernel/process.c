#include "process.h"
#include "schedule.h"
#include "../drivers/qemu/timer.h"
#include "libk/includes/stdlib.h"
#include "libk/includes/string.h"
#include "libk/includes/stdio.h"
#include "global.h"

uint64 next_pid = 1;

void task_trampoline() {
  enable_interrupts();
  void (*entry)(void) = (void (*)(void))current_task->ctx.x19;
  entry();
  kexit();
}

struct task* task_create(void (*entry)(void)) {
  struct task* new_task = kmalloc(sizeof(struct task));
  memset(new_task, 0, sizeof(struct task));
  new_task->stack_base = kmalloc(STACK_SIZE);
  new_task->pid = next_pid++;
  new_task->state = READY;

  uint64 sp_top = ((uint64) new_task->stack_base + STACK_SIZE) & ~0xF;

  memset(&new_task->ctx, 0, sizeof(struct context));
  new_task->ctx.x30 = (uint64)task_trampoline;
  new_task->ctx.x19 = (uint64)entry; 
  new_task->ctx.sp = sp_top;

  return new_task;
}

void task_free(struct task* t){
  kfree(t->stack_base);
  kfree(t);
}
