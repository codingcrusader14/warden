#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include "../drivers/qemu/pl011.h" 
#include "libk/includes/stdio.h"

void kernel_main(void) {
   pl011 uart;
   pl011_setup_qemu(&uart); 
   
   asm volatile("svc #0");
}
