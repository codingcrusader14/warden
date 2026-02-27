#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include "../drivers/qemu/pl011.h" 
#include "libk/includes/stdio.h"
#include "libk/includes/stdlib.h"
#include "vmm.h"
#include "pmm.h"

void kernel_main(void) {
   uart_init();
   pmm_init(QEMU_DRAM_START, QEMU_DRAM_END);
   vmm_init();
   int32* num = 0x0;
   kprintf("The value of num is %d\n", *num);
   
}
