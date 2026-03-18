#include "trap.h"
#include "../drivers/qemu/gic.h"
#include "../drivers/qemu/pl011.h"
#include "../drivers/qemu/timer.h"
#include "global.h"
#include "libk/includes/stdio.h"
#include "schedule.h"
#include "syscall.h"
#include "types.h"

void kernelvec_sync(struct trapframe *tf) {
  current_task->tf = tf;
  uint64 ec = (tf->esr_el1 >> 26) & (0x3F);
  switch (ec) {
  case 0x15: { // system calls
    uint64 sys_num = tf->x8;
    switch (sys_num) {
    case SYS_EXIT: {
      kprintf("SYS_EXIT from pid %d\n", current_task->pid);
      tf->x0 = handle_sys_exit(tf->x0);
      break;
    }

    case SYS_YIELD: {
      kprintf("SYS_YIELD from pid %d\n", current_task->pid);
      handle_sys_yield();
      break;
    }

    case SYS_WRITE: {
      int fd = (int)tf->x0;
      const void *buf = (const char *)tf->x1;
      size_t len = tf->x2;
      tf->x0 = handle_sys_write(fd, buf, len);
      break;
    }

    case SYS_READ: {
      int fd = (int)tf->x0;
      void *buf = (void *)tf->x1;
      size_t len = tf->x2;
      tf->x0 = handle_sys_read(fd, buf, len);
      break;
    }

    case SYS_GETPID: {
      tf->x0 = handle_getpid();
      break;
    }

    case SYS_FORK: {
      tf->x0 = handle_fork();
      break;
    }

    case SYS_WAIT: {
      int* status = (int*)tf->x0;
      tf->x0 = handle_wait(status);
      break;
    }

    case SYS_SBRK: {
      int incr = (int)tf->x0;
      tf->x0 = (uint64)handle_sbrk(incr);
      break;
    }

    default: {
      kprintf("Syscall does not exist\n");
      break;
    }
    }
    break;
  }

  default: {
    kprintf("Exception SYNC Recieved!\n");
    kprintf("Exception SYNC from pid %d\n", current_task->pid);
    kprintf("Exception (ESR EC Code) : %d\n", ec);
    kprintf("Faulting Address (FAR): %p\n", tf->far_el1);
    kprintf("Return Address (ELR): %p\n", tf->elr_el1);
    while (1) {
    }
  }
  }
}
void kernelvec_irq(struct trapframe *tf) {
  current_task->tf = tf;
  uint32 ack_id = read_interrupt_ack();
  switch (ack_id) {
  case 30: {
    ;
    timer_rearm();
    write_end_of_interrupt(ack_id);
    schedule();
    break;
  }

  case 33: {
    uart_isr();
    write_end_of_interrupt(ack_id);
    break;
  }

  default:
    kprintf("This interrupt is not recongnized\n");
    break;
  }
}

void kernelvec_fiq(struct trapframe *tf) {
  kprintf("Exception FIQ Recieved!\n");
  kprintf("Faulting Address (FAR): %p\n", tf->far_el1);
  while (1) {
  }
}

void kernelvec_system(struct trapframe *tf) {
  kprintf("Exception SERROR Recieved!\n");
  kprintf("Faulting Address (FAR): %p\n", tf->far_el1);
  while (1) {
  }
}
