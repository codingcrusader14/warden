#include "syscall.h"
#include "../drivers/qemu/pl011.h"
#include "libk/includes/stdio.h"
#include "mmu_defs.h"
#include "process.h"
#include "schedule.h"
#include "file.h"
#include "trap.h"
#include "types.h"
#include "vmm.h"
#include "pipe.h"
#include "pmm.h"
#include "wait_queue.h"

extern void fork_return();

int handle_sys_exit(int status) {
  current_task->exit_status = status;
  current_task->state = ZOMBIE;

  if (current_task->parent) {
    lock(&current_task->parent->child_wq.spinlock);
    wait_queue_wakeup(&current_task->parent->child_wq);
    unlock(&current_task->parent->child_wq.spinlock);
  }
  schedule();
  kprintf("Panic zombie scheduled\n");
  return 0;
}

void handle_sys_yield() { yield(); }

ssize_t handle_sys_write(int fd, const void* buf, size_t len) {
  if (!buf || len == 0) 
    return 0;

  file* f_write = fd_to_file(current_task, fd);
  if (!f_write) {
    return 0;
  }
  
  return f_write->ops->write(f_write, buf, len);
}

ssize_t handle_sys_read(int fd, void *buf, size_t len) {
  if (!buf || len == 0)
    return 0;

  file* f_read = fd_to_file(current_task, fd);
  if (!f_read) {
    return 0;
  }

  return f_read->ops->read(f_read, buf, len);
}

pid_t handle_getpid() { return current_task->pid; }

pid_t handle_fork() {
  task_t* parent = current_task;
  task_t* child = task_alloc(parent->tickets); // copy parents priority within scheduler

    // copy parent file descriptor entries 
  for (size_t i = 0; i < MAX_FDS; ++i) {
    if (parent->fd_table[i]) {
      child->fd_table[i] = parent->fd_table[i];
      file_ref(child->fd_table[i]);
    }
  }

  child->pass = parent->pass;
  child->remain = parent->remain;
  child->scheduler_tick = parent->scheduler_tick;
  child->brk = parent->brk;
  child->pgd = alloc_page_table(); 
  copy_user_pagetable((pte_t*) PA_TO_KVA(parent->pgd), (pte_t*) PA_TO_KVA(child->pgd));
  child->parent = parent;
  child->sibling = parent->children;
  parent->children = child;

  // place trapframe at top of child's kernel stack
  uint64 child_sp_top = ((uint64) child->kstack + STACK_SIZE) & ~0xF;
  trapframe *child_tf = (trapframe*)(child_sp_top - sizeof(trapframe));
  *child_tf = *(parent->tf);
  child_tf->x0 = 0;
  child->tf = child_tf;

  // set up context so scheduler can switch to the child
  child->ctx.sp = (uint64)child_tf;
  child->ctx.x30 = (uint64)fork_return;

  // add to scheduler
  scheduler_add(child);

  return child->pid;
}

pid_t handle_wait(int* status) {
  if (!current_task->children) return -1;

  while (1) {
    lock(&current_task->child_wq.spinlock);

    task_t* prev = NULL;
    task_t* curr = current_task->children;
    while (curr) {
      if (curr->state == ZOMBIE) {
        // unlink
        if (prev) {
            prev->sibling = curr->sibling;
        }
        else {
          current_task->children = curr->sibling;
        }
        pid_t child_pid = curr->pid;
        if (status) 
          *status = curr->exit_status;

        unlock(&current_task->child_wq.spinlock);
        task_free(curr);
        return child_pid;
        
      }
      prev = curr;
      curr = curr->sibling;
    }
    wait_queue_sleep(&current_task->child_wq, &current_task->child_wq.spinlock);
    unlock(&current_task->child_wq.spinlock);
  }
  return -1; 
}

void* handle_sbrk(int incr) {
  
  uint64 old_brk = current_task->brk;
  uint64 new_brk = old_brk + incr;
  kprintf("sbrk: old_brk=%p new_brk=%p incr=%d\n", old_brk, new_brk, incr);

  if (incr > 0) {
    for (uint64 addr = old_brk; addr < new_brk; addr += PAGE_SIZE) {
      pa_t page = (pa_t)pmm_alloc();
      if (!page) return (void*)-1;
      map_page((pa_t*) PA_TO_KVA(current_task->pgd), addr, page, USER_FLAGS);
    }
  } else if (incr < 0) {
    for (uint64 addr = new_brk; addr < old_brk; addr += PAGE_SIZE) {
      unmap_page((pa_t*) PA_TO_KVA(current_task->pgd) , addr);
    }
  }
  current_task->brk = new_brk;
  return (void*)old_brk;
}

int handle_close(int fd) {
  file* f_close = fd_to_file(current_task, fd);
  if (!f_close) {
    return 0;
  }
  current_task->fd_table[fd] = NULL;
  int64 rc = file_close(f_close);
  return rc;
}

static void pipe_error_cleanup(file* pipe_fw, file* pipe_fr) {
    file_close(pipe_fw);
    file_close(pipe_fr);
}

int handle_pipe(int p[]) {
  pipe* new_pipe = pipe_alloc();
  if (!new_pipe) 
    return -1;

  file_ops* pipe_write = &pipe_write_ops;
  file_ops* pipe_read = &pipe_read_ops;
  file* pipe_fw = file_alloc(FILE_PIPE_WRITE, pipe_write, new_pipe);
  file* pipe_fr = file_alloc(FILE_PIPE_READ, pipe_read, new_pipe);

  if (!pipe_fw || !pipe_fr) {
    if (pipe_fw) 
      file_close(pipe_fw);

    if (pipe_fr)
      file_close(pipe_fr);
    return -1;
  }

  int32 fd_read = find_free_fd(current_task, pipe_fr);
  if (fd_read == -1) {
    pipe_error_cleanup(pipe_fw, pipe_fr);
    return -1;
  }
  int32 fd_write = find_free_fd(current_task, pipe_fw);
   if (fd_write == -1) {
    current_task->fd_table[fd_read] = NULL;
    pipe_error_cleanup(pipe_fw, pipe_fr);
    return -1;
  }

  int fds[] = {fd_read, fd_write};

   if (copy_to_user((pte_t *)PA_TO_KVA(current_task->pgd), p, fds, sizeof(int) * 2) < 0) {
    current_task->fd_table[fd_write] = NULL;
    current_task->fd_table[fd_read] = NULL;
    pipe_error_cleanup(pipe_fw, pipe_fr);
    return -1;
   }

  return 0;
}
