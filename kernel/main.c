#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include "../drivers/qemu/pl011.h" 
#include "../drivers/qemu/timer.h"
#include "../drivers/qemu/gic.h"
#include "libk/includes/stdio.h"
#include "vmm.h"
#include "pmm.h"
#include "schedule.h" 
#include "process.h"
#include "spinlock.h"

static volatile uint64 counter = 0;
static lock_t mutex;

void task_a(void) {
    kprintf("Entering Task \n");
    for (size_t i = 0; i < 10000000; ++i) {
      lock(&mutex);
      counter += 1;
      unlock(&mutex);
    }
    kprintf("Finishing Task Counter Val %d\n", counter);
}


void kernel_main(void) {
   uart_init();
   pmm_init(QEMU_DRAM_START, QEMU_DRAM_END);
   vmm_init();
   gic_init();
   timer_init();
   scheduler_init();
   task_t* a = task_create(task_a, NORMAL_TASK);
   task_t* b = task_create(task_a, NORMAL_TASK);
   scheduler_add(a);
   scheduler_add(b);
   scheduler_start();


  


}
