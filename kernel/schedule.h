#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "process.h"

#define MAX_TASKS 256
#define STRIDE_BASE (1 << 20)
#define QUANTUM (1 << 20)

typedef struct {
  task_t* processes[MAX_TASKS];
  size_t size;
} scheduler_t; 

typedef struct {
  task_t* dead_task[MAX_TASKS];
  size_t size;
} zombie_t;

extern task_t* current_task;
extern zombie_t cleanup;

void cleanup_dead_task();
void scheduler_add(task_t *t);
void scheduler_pop();
int schedule();
void scheduler_init();
void scheduler_boot();
int scheduler_start();


#endif
