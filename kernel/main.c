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
   debug_va(kernel_L0, 0x09000000);
   kprintf("\n");
   debug_va(kernel_L0, 0x40080000);
   kprintf("\n");
   debug_va(kernel_L0, (va_t)QEMU_DRAM_START + 0x10000);
   unmap_page(kernel_L0, (va_t) QEMU_DRAM_START + 0x10000);
   kprintf("\n");
   debug_va(kernel_L0, (va_t)QEMU_DRAM_START + 0x10000);
   debug_va(kernel_L0, 0x0);
}
