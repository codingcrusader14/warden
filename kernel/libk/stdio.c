#include "stdio.h"
#include "../../drivers/qemu/pl011.h"
#include "string.h"
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>

static const char hex_values[] = {'0', '1', '2', '3', '4', '5',
                '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

static void reverse_buffer(char* buf, int buf_len, int start) {
  int l = start, r = buf_len;
  while (l <= r) {
    char temp = buf[l];
    buf[l] = buf[r];
    buf[r] = temp;
    l++;
    r--;
  }
}

int kprintf(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  while (*format != '\0') {
    char character = *format;

    switch (character) {
      case '%':
        format++;
        switch (*format) {
          case 'd': // int
          case 'i': { // int  
            int num = va_arg(ap, int);
            char num_buffer[12];
            bool negative = false;
            int i = 0;

            if (num == 0) { // empty case
              put_char('0');
              break;
            } 
            
            if (num < 0) { // negative case
              negative = true;
              num_buffer[i++] = '-';
            }

            unsigned int unum = (negative) ? -(unsigned int)num : (unsigned int)num;
            while (unum) {
              char digit = unum % 10;
              num_buffer[i++] = digit + '0';
              unum /= 10;
            }

            num_buffer[i] = '\0';
            int buffer_len = i - 1;
            reverse_buffer(num_buffer, buffer_len, (negative) ? 1 : 0);
            send_message(num_buffer);
            break;
          }
          case 's': { // string
            const char* str = va_arg(ap, char*);
            send_message(str);
            break;
          }
          case 'x': { // hex
            unsigned int num = va_arg(ap, unsigned int);
            char hex_buffer[9]; 
            if (num == 0) {
              put_char('0');
              break;
            }

            int i = 0;
            while (num) { 
              int hex = num % 16; 
              hex_buffer[i++] = hex_values[hex];
              num /= 16;
            }

            hex_buffer[i] = '\0';
            int buffer_len = i - 1;
            reverse_buffer(hex_buffer, buffer_len, 0);
            send_message(hex_buffer);
            break;
          }
          case 'p': { // pointer
            void* p = va_arg(ap, void*);
            uintptr_t ptr_val = (uintptr_t)p; 
            char ptr_buffer[19] = {'0','x'};
            int i = 2;

            if (ptr_val == 0) {
              ptr_buffer[i++] = '0';
              ptr_buffer[i] = '\0';
              send_message(ptr_buffer);
              break;
            }

            while (ptr_val) { 
              int hex = ptr_val % 16; 
              ptr_buffer[i++] = hex_values[hex];
              ptr_val /= 16;
            }

            ptr_buffer[i] = '\0';
            int buffer_len = i - 1;
            reverse_buffer(ptr_buffer, buffer_len, 2);
            send_message(ptr_buffer);
          } break;

          case '%': // display % symbol
            put_char('%');
            break;

          default: // format not specified
            put_char('%');
            put_char(*format);
            break;
        } 
        format++;
        break;

      default: // plain characters
        put_char(character);
        format++;
        break;
    }
  }
  va_end(ap);
  return 0;
}
