#include <stdint.h>
#include <stddef.h>
#include "../drivers/qemu/pl011.h" 




void kernel_main(void) {
   pl011 uart;
   pl011_setup_qemu(&uart); 
   
   send_message("hello world!\n");
   send_message("hello world!\n");
   const char* str = "hello world!\n";

   while (*str) {
    put_char(*str);
    str++;
   }

   char buffer[100];
   int c, i = 0;
   while ((c = get_char()) != '\r') {
    buffer[i++] = c;
   }
   buffer[i] = '\0';
   send_message(buffer);
}
