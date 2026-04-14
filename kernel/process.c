#include "process.h"
#include "../drivers/qemu/timer.h"
#include "elf.h"
#include "file.h"
#include "global.h"
#include "libk/includes/stdio.h"
#include "libk/includes/string.h"
#include "libk/includes/stdlib.h"
#include "../fs/fat32.h"
#include "mmu_defs.h"
#include "pmm.h"
#include "schedule.h"
#include "spinlock.h"
#include "types.h"
#include "vmm.h"

uint64 next_pid = 1;
cpu cpus[NCPU];

extern void enter_userspace(uint64 pgd, const void *entry, void *ustack);

void user_entry() {
  enter_userspace((uint64)current_task->pgd, current_task->entry_point,
                  current_task->ustack);
}

int kexit() {
  current_task->state = DEAD;
  int ret = schedule();
  if (ret == -1) {
    kprintf("All tasks complete. Halting.\n");
    disable_interrupts();
    while (1)
      ;
  }
  kprintf("Panic: kexit should not return.\n");
  while (1)
    ;
}

void yield() {
  current_task->state = READY;
  schedule();
}

void sleep(lock_t *mutex) {
  current_task->state = BLOCKED;
  unlock(mutex);
  enable_interrupts();
  schedule();
  disable_interrupts();
  lock(mutex);
}

void wakeup(task_t *t) {
  if (t && t->state == BLOCKED) {
    t->state = READY;
    scheduler_add(t);
  }
}

void task_trampoline() {
  enable_interrupts();
  void (*entry)(void *) = (void (*)(void *))current_task->ctx.x19;
  void *arg = (void *)current_task->ctx.x20;
  entry(arg);
  kexit();
}

task_t *task_alloc(uint64 ticket_level) {
  task_t *new_task = kmalloc(sizeof(task_t));
  memset(new_task, 0, sizeof(task_t));
  new_task->kstack = kmalloc(STACK_SIZE);
  new_task->pid = next_pid++;
  new_task->state = READY;
  new_task->tickets = ticket_level;
  new_task->stride = (STRIDE_BASE / new_task->tickets);
  new_task->pass = 0;
  new_task->remain = new_task->stride;
  new_task->scheduler_tick = 0;
  new_task->next_wait = NULL;
  new_task->parent = NULL;
  new_task->children = NULL;
  new_task->sibling = NULL;
  new_task->cwd_cluster = root_cluster;
  wait_queue_init(&new_task->child_wq);
  return new_task;
}

task_t *kernel_task_create(void (*entry)(void*), void* args, uint64 ticket_level) {
  task_t *new_task = task_alloc(ticket_level);
  uint64 sp_top = ((uint64)new_task->kstack + STACK_SIZE) & ~0xF;
  memset(&new_task->ctx, 0, sizeof(context));
  new_task->ctx.x30 = (uint64)task_trampoline;
  new_task->ctx.x19 = (uint64)entry;
  new_task->ctx.x20 = (uint64)args;
  new_task->ctx.sp = sp_top;
  new_task->pgd = NULL;
  new_task->ustack = NULL;
  new_task->entry_point = NULL;
  return new_task;
}

task_t *task_create(void (*entry)(void *), void *args, uint64 ticket_level) {
  task_t *new_task = task_alloc(ticket_level);
  uint64 sp_top = ((uint64)new_task->kstack + STACK_SIZE) & ~0xF;
  memset(&new_task->ctx, 0, sizeof(context));
  new_task->ctx.x30 = (uint64)task_trampoline;
  new_task->ctx.x19 = (uint64)entry;
  new_task->ctx.x20 = (uint64)args;
  new_task->ctx.sp = sp_top;

  /* allocate user page table */
  new_task->pgd = (uint64 *)alloc_page_table();

  /* map elf file */
  extern char _user_elf_start[];
  extern char _user_elf_end[];
  uint64 size = (uint64)(_user_elf_end - _user_elf_start);
  uint64 entry_point = parse_and_map_elf(_user_elf_start, size, new_task);
  if (!entry_point) {
    kprintf("ELF load failed\n");
    return NULL;
  }

  /* map user stack */
  pa_t stack = (pa_t)pmm_alloc(); // user stack segment
  map_page(new_task->pgd, USER_STACK - PAGE_SIZE, stack,
           USER_FLAGS); // stack grows down
  new_task->ustack = (void *)USER_STACK;
  new_task->entry_point = (void *)entry_point;
  return new_task;
}

void task_free(task_t *t) {
  if (t->pgd) {
    pte_t* kva_pgd = (pte_t*)PA_TO_KVA(t->pgd);
    free_user_pages(kva_pgd); 
    pmm_free((pa_t*)t->pgd); // free page table
  }
  kfree(t->kstack);
  kfree(t);
}

int32 find_free_fd(task_t* t, file* f) {
  if (!t || !f) return -1;

  file** fd_table = t->fd_table;
  for (int32 i = 0; i < MAX_FDS; ++i) {
    if (!fd_table[i]) {
      fd_table[i] = f;
      return i;
    }
  }
  return -1;
}

file* fd_to_file(task_t* t, int32 fd) {
  if (fd >= 0 && fd < MAX_FDS) {
    return t->fd_table[fd];
  }
  return NULL;
}
