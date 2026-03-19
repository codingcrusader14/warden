#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "file.h"
#include "global.h"
#include "spinlock.h"
#include "trap.h"
#include "wait_queue.h"

extern uint64 next_pid;

#define REAPER_TASK 5
#define NORMAL_TASK 100

enum task_state {
  READY,
  RUNNING,
  BLOCKED,
  DEAD,
  ZOMBIE,
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

typedef struct task{
  uint64 pid; 
  enum task_state state;
  int exit_status;
  uint64 tickets;
  uint64 stride;
  uint64 remain;
  uint64 pass;
  uint64 scheduler_tick;
  trapframe* tf; // trapframe
  void* kstack; // kernel stack for this process
  void* ustack; // user stack physical base
  uint64 code_size; // total size of program 
  uint64 brk; // top address of heap
  uint64* pgd; // physical address of TTBR0 L0 page table
  context ctx; // switch() runs on this
  void* entry_point;
  struct task* next_wait;
  struct task* parent; // needed for fork implementation
  struct task* children;
  struct task* sibling;
  wait_queue child_wq;
  file* fd_table[MAX_FDS]; // file descriptor table
} task_t;

typedef struct {
  task_t* current_task; 
  context ctx; 
  uint32 exception_state; // what was the debug exception, serror, irq, or fiq before acquiring lock
  uint32 nested_locks; // 1 on unlock enables intterupts, anything >= 1 means nested lock sceario and we dont touch exception state on > 1 locks
  uint32 id;
} cpu; 

extern cpu cpus[NCPU];

void enter_userspace(uint64 pgd, const void* entry, void* ustack);
void user_entry();
void sleep(lock_t* mutex);
void wakeup(task_t* t);
int kexit();
void yield();
void task_trampoline();
task_t* task_alloc(uint64 ticket_level);
task_t* task_create(void (*entry)(void*), void* args, uint64 ticket_level);
void task_free(task_t* t);
int32 find_free_fd(task_t* t, file* f);
file* fd_to_file(task_t* t, int32 fd);

#endif
