#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include "../drivers/qemu/pl011.h" 
#include "libk/includes/stdio.h"

void kernel_main(void) {
   pl011 uart;
   pl011_setup_qemu(&uart); 
   const char* me = "Andrew";
   int age = 23;
   kprintf("My name is %s and my age is %d.", me, age);
}
