#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include "../drivers/qemu/pl011.h" 
#include "libk/includes/stdio.h"
#include "libk/includes/stdlib.h"
#include "../drivers/qemu/timer.h"
#include "../drivers/qemu/gic.h"
#include "vmm.h"
#include "pmm.h"

void kernel_main(void) {
   uart_init();
   pmm_init(QEMU_DRAM_START, QEMU_DRAM_END);
   vmm_init();
   gic_init();
   timer_init();
   
}
