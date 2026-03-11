#include <stdbool.h>
#include "../drivers/qemu/timer.h"
#include "libk/includes/stdio.h"
#include "schedule.h"
#include "process.h"
#include "stddef.h"
#include "clock.h"

void write_ttbr0_el1(uint64 physical_address){
  asm volatile("msr ttbr0_el1, %0" :: "r"(physical_address));
}

extern void context_switch(context* old, context* new);
extern void tlb_invalidate_process(va_t virtual_address);

static scheduler_t scheduler; // Fair Share Stride Scheduler - Only contains ready processes
zombie_t cleanup; // Dead tasks for cleanup 
static uint64 global_tickets, global_stride, global_pass;
task_t* current_task = NULL;

void cleanup_dead_task() {
  for (size_t i = 0; i < cleanup.size; ++i) {
    task_free(cleanup.dead_task[i]);
    cleanup.dead_task[i] = NULL;
  }
  cleanup.size = 0;
}

static void global_pass_update() {
  static uint64 last_update = 0;

  uint64 elapsed = get_clock_ticks() - last_update;
  last_update += elapsed;

  global_pass += (global_stride * elapsed) / QUANTUM;
}

void global_tickets_update(uint64 delta) {
  global_tickets += delta;
  global_stride = (STRIDE_BASE / global_tickets);
}

static task_t* scheduler_min_pass() {
  if (scheduler.size == 0) {
    kprintf("No ready elements waiting to be scheduled\n");
    return NULL;
  }
  return scheduler.processes[0]; // return task with least pass value
}

void scheduler_add(task_t *t) {
  if (t == NULL) return; // bad task

  global_pass_update();
  t->remain = t->pass - global_pass; 
  global_tickets_update(t->tickets);
  scheduler.processes[scheduler.size++] = t;
  size_t i = scheduler.size - 1;
  while (i > 0 && scheduler.processes[i]->pass < scheduler.processes[(i - 1)/ 2]->pass) { // perlocate up the lower pass value
    task_t* temp = scheduler.processes[i];
    scheduler.processes[i] = scheduler.processes[(i - 1)/ 2];
    scheduler.processes[(i - 1) / 2] = temp;
    i = (i - 1) / 2;
 }
}

void scheduler_pop() {
  size_t size = scheduler.size;

  if (size > 1) {
    task_t* temp = scheduler.processes[0];
    scheduler.processes[0] = scheduler.processes[size - 1];
    scheduler.processes[size - 1] = temp;

    scheduler.processes[size - 1] = NULL;
    scheduler.size--;
    size--;

    size_t i = 0;

    while (2 * i + 1 < size) {
      size_t left = 2 * i + 1;
      size_t right = 2 * i + 2;

      size_t smallest = left;
      if (right < size && scheduler.processes[right]->pass < scheduler.processes[left]->pass) { 
        smallest = right;
      }

      if (!(scheduler.processes[smallest]->pass < scheduler.processes[i]->pass)) break;
      
      task_t* temp = scheduler.processes[i]; // perlocate down
      scheduler.processes[i] = scheduler.processes[smallest];
      scheduler.processes[smallest] = temp;
      i = smallest;
    }
  } else if (size == 1) {
    scheduler.processes[size - 1] = NULL;
    scheduler.size--;
  }
}

void scheduler_init() {
  scheduler.size = 0;
  cleanup.size = 0;
}

int schedule() {
  if (!current_task) return -1;
  
  uint64 elapsed_time = get_clock_ticks() - current_task->scheduler_tick;
  current_task->pass += (current_task->stride * elapsed_time) / QUANTUM;
  if (current_task->state == READY) {
    scheduler_add(current_task);
  }

  if (current_task->state == DEAD) {
    cleanup.dead_task[cleanup.size] = current_task;
    cleanup.size++;
  }

  bool find_next;
  task_t* next_task;
  do { 
    find_next = false;
    next_task = scheduler_min_pass();
    if (next_task == NULL) return -1; // no tasks left
    
    global_pass_update();
    next_task->remain = next_task->pass - global_pass;
    global_tickets_update(-next_task->tickets);
    scheduler_pop();

    if (next_task->state == DEAD) {
      cleanup.dead_task[cleanup.size] = next_task;
      cleanup.size++;
      find_next = true;
    }
  } while (find_next);

  task_t* old_task = current_task;
  current_task = next_task;
  
  next_task->scheduler_tick = get_clock_ticks();
  write_ttbr0_el1((uint64) next_task->pgd);
  tlb_invalidate_process((va_t) 0); // asid not yet implemented
  context_switch(&old_task->ctx, &next_task->ctx);
  return 0;
}


int scheduler_start() {
  disable_interrupts();
  current_task = scheduler_min_pass();
  if (!current_task) {
    kprintf("No tasks are available\n");
    return -1;
  }
  task_t dummy;
  scheduler_pop();
  current_task->scheduler_tick = get_clock_ticks();
  context_switch(&dummy.ctx, &current_task->ctx);
  return 0;
}




