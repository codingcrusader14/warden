#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "global.h"

extern uint64 next_pid;

#define REAPER_TASK 5
#define NORMAL_TASK 100

enum task_state {
  READY,
  RUNNING,
  BLOCKED,
  DEAD,
};

typedef struct {
  uint64 x19;
  uint64 x20;
  uint64 x21;
  uint64 x22;
  uint64 x23;
  uint64 x24;
  uint64 x25;
  uint64 x26;
  uint64 x27;
  uint64 x28;
  uint64 x29;
  uint64 x30; // return address
  uint64 sp; // stack pointer
} context;

typedef struct {
  context ctx;
  uint64 pid; 
  enum task_state state;
  uint64 tickets;
  uint64 stride;
  uint64 remain;
  uint64 pass;
  uint64 scheduler_tick;
  void* stack_base; // base address of stack
} task_t; 

typedef struct {
  task_t* current_task; 
  context ctx; 
  uint32 exception_state; // what was the debug exception, serror, irq, or fiq before acquiring lock
  uint32 nested_locks; // 1 on unlock enables intterupts, anything >= 1 means nested lock sceario and we dont touch exception state on > 1 locks
  uint32 id;
} cpu; 

extern cpu cpus[NCPU];

void kexit();
void yield();
void task_trampoline();
task_t* task_create(void (*entry)(void), uint64 ticket_level);
void task_free(task_t* t);
#endif
