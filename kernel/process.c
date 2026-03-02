#include "process.h"
#include "libk/includes/stdlib.h"
#include "libk/includes/string.h"
#include "global.h"

uint64 next_pid = 1;

struct task* task_create(void (*entry)(void)) {
  struct task* new_task = kmalloc(sizeof(struct task));
  memset(new_task, 0, sizeof(struct task));
  new_task->stack_base = kmalloc(STACK_SIZE);
  new_task->pid = next_pid++;
  new_task->state = READY;
  new_task->context_registers.x30 = (uint64) entry;
  new_task->context_registers.sp = ((uint64) new_task->stack_base + STACK_SIZE) & ~0xF;
  return new_task;
}
