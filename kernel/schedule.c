#include "schedule.h"
#include "process.h"
#include "stddef.h"
#include "../drivers/qemu/timer.h"
#include "libk/includes/stdio.h"


extern void context_switch(struct context* old, struct context* new);

static struct task* ready_queue = NULL;
struct task* current_task = NULL;
static struct task* prev_task = NULL;
static struct task* last_task = NULL;
static struct task* zombie = NULL;

void kexit() {
  current_task->state = DEAD;
  int ret = schedule();
  if (ret == -1) {
      kprintf("All tasks complete. Halting.\n");
      disable_interrupts();
      while(1);
    }
    kprintf("Panic: kexit should not return.\n");
    while(1);
}

void yield() {
  current_task->state = READY;
  schedule();
}


void scheduler_add(struct task *t) {
  if (t == NULL) return; 

  if (!ready_queue) { // no active tasks
    ready_queue = t;
    t->next_task = t;
    current_task = ready_queue;
    prev_task = current_task;
    last_task = current_task;
  } else {
    t->next_task = ready_queue; // add newest ready task pointing to oldest ready
    last_task->next_task = t; // move the previous task pointing to the newest one
    last_task = last_task->next_task; // move the last task pointing to new task
  }
}

int schedule() {
  if (zombie) {
    task_free(zombie);
    zombie = NULL;
  }
  if (!current_task) 
    return -1;

  struct task* start = current_task;
  prev_task = current_task;
  current_task = current_task->next_task;
  do {
    if (current_task->state == DEAD) {
      if (last_task == current_task) {
        last_task = prev_task;
      }
      prev_task->next_task = current_task->next_task;
      struct task* temp = current_task;
      current_task = current_task->next_task;
      task_free(temp);
    } else {
      break;
    }
  } while (start != current_task);

  if (start == current_task) { 
    if (current_task->state == DEAD) {
      zombie = current_task;
      ready_queue = NULL;
      current_task = NULL;
      prev_task = NULL;
      last_task = NULL;
      return -1; // no valid task
    } else {
      return 1; // same task
    }
  } 
  if (start->state == DEAD) {
    struct task* p = current_task;
    while (p->next_task != start) {
      p = p->next_task;
    }
    p->next_task = start->next_task;
    if (ready_queue == start) ready_queue = start->next_task;
    if (last_task == start) last_task = p;
    zombie = start;
  }
  context_switch(&start->ctx, &current_task->ctx);
  return 0; // context switch
}

int scheduler_start() {
  disable_interrupts();
  if (!current_task) {
    kprintf("No tasks in system\n");
    return -1;
  }
  struct task dummy;
  context_switch(&dummy.ctx, &current_task->ctx);
  return 0;
}

