#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

extern uint64 next_pid;

enum task_state {
  READY,
  RUNNING,
  BLOCKED,
  DEAD,
};

struct context {
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
};

struct task {
  struct context ctx;
  uint64 pid; 
  enum task_state state;
  struct task* next_task;
  void* stack_base; // base address of stack
}; 

void task_trampoline();
struct task* task_create(void (*entry)(void));
void task_free(struct task* t);
#endif
