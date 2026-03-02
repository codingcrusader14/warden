#include "schedule.h"
#include "process.h"
#include "stddef.h"
#include "libk/includes/stdio.h"

extern void context_switch(struct task_context* prev, struct task_context* next);
static struct task* ready_queue = NULL;
struct task* current_task = NULL;

void schedule() {
  struct task* prev_task = current_task;
  current_task = current_task->next_task;
  if (current_task == prev_task) 
    return; 
  prev_task->state = READY;
  current_task->state = RUNNING;
  context_switch(&prev_task->context_registers, &current_task->context_registers);
}

void scheduler_add(struct task *t) {
  if (!ready_queue) {
    ready_queue = t;
    t->next_task = t;
    current_task = ready_queue;
  } else {
    t->next_task = ready_queue->next_task;
    ready_queue->next_task = t;
  }
}
