#include "../drivers/qemu/gic.h"
#include "../drivers/qemu/timer.h"
#include "libk/includes/stdio.h"
#include "trap.h" 
#include "syscall.h"
#include "types.h"
#include "global.h"
#include "schedule.h"

void kernelvec_sync(struct trapframe *tf) {
  uint64 ec = (tf->esr_el1 >> 26) & (0x3F);
  switch (ec) {
    case 0x15 : { // system calls
      uint64 sys_num = tf->x8; 
      switch (sys_num) {
        case SYS_EXIT : {
          kprintf("SYS_EXIT from pid %d\n", current_task->pid);
          handle_sys_exit(tf->x0);
          break;
        } 

        case SYS_YIELD : {
          kprintf("SYS_YIELD from pid %d\n", current_task->pid);
          handle_sys_yield(); 
          break;
        }

        case SYS_WRITE : {
          const char* buf = (const char*)tf->x0;
          size_t len = tf->x1;
          handle_sys_write(buf, len);
          break;
        }

        default : {
          kprintf("Syscall does not exist\n");
          break;
        }
      }
      break;
    }
    
    default : {
      kprintf("Exception SYNC Recieved!\n");
      kprintf("Exception SYNC from pid %d\n", current_task->pid);
      kprintf("Exception (ESR EC Code) : %d\n", ec);
      kprintf("Faulting Address (FAR): %p\n", tf->far_el1);
      kprintf("Return Address (ELR): %p\n", tf->elr_el1);
      while (1) {}
    }

  }
}
void kernelvec_irq(struct trapframe* tf) {
  UNUSED(tf);
  uint32 ack_id = read_interrupt_ack();
  switch(ack_id) {
    case 30 : {;
      timer_rearm();
      write_end_of_interrupt(ack_id);
      schedule();
      break;
    }
    default : 
      kprintf("This interrupt is not recongnized\n");
      break;
  }
}

void kernelvec_fiq(struct trapframe *tf) {
  kprintf("Exception FIQ Recieved!\n");
  kprintf("Faulting Address (FAR): %p\n", tf->far_el1);
  while (1) {}
}

void kernelvec_system(struct trapframe *tf) {
   kprintf("Exception SERROR Recieved!\n");
   kprintf("Faulting Address (FAR): %p\n", tf->far_el1);
   while (1) {}
}
